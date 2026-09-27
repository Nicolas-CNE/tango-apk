#include "install-apk.hpp"
#include "rogersat-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

namespace RogerAPK {

static std::map<std::string, PackageSpec> loadRepoIndex(const std::string& root_dir) {
    std::map<std::string, PackageSpec> db;
    fs::path index_path = fs::path(root_dir) / "var/lib/roger-apk/repo_index";

    if (!fs::exists(index_path)) return db;

    std::ifstream file(index_path);
    std::string line;
    PackageSpec current;

    while (std::getline(file, line)) {
        if (line == "---") {
            if (!current.name.empty()) db[current.name] = current;
            current = PackageSpec();
            continue;
        }
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t\r\n") + 1);

            if (key == "pkgname") current.name = val;
            else if (key == "version") current.version = val;
            else if (key == "size") current.size = std::stoull(val);
            else if (key == "installed_size") current.installed_size = std::stoull(val);
            else if (key == "depends") {
                std::string dep;
                std::istringstream ss(val);
                while (ss >> dep) current.depends.push_back(dep);
            }
        }
    }
    return db;
}

void cmd_install_apk(const std::string& root_dir, const std::vector<std::string>& targets) {
    if (targets.empty()) {
        CLI_APK::printError("Debe especificar al menos un paquete para instalar.");
        return;
    }

    auto repo_db = loadRepoIndex(root_dir);
    if (repo_db.empty()) {
        CLI_APK::printError("El índice de repositorios está vacío. Ejecute 'roger-apk sync' primero.");
        return;
    }

    DependencyResolverSAT resolver;
    for (const auto& [name, pkg] : repo_db) {
        resolver.addPackage(pkg);
    }

    std::vector<std::string> install_order;
    for (const auto& target : targets) {
        std::vector<std::string> sub_list;
        if (!resolver.resolveInstall(target, sub_list)) {
            CLI_APK::printError("No se pudieron resolver las dependencias para: " + target);
            return;
        }
        for (const auto& pkg : sub_list) {
            if (std::find(install_order.begin(), install_order.end(), pkg) == install_order.end()) {
                install_order.push_back(pkg);
            }
        }
    }

    std::vector<std::string> deps_pkgs;
    for (const auto& pkg : install_order) {
        if (std::find(targets.begin(), targets.end(), pkg) == targets.end()) {
            deps_pkgs.push_back(pkg);
        }
    }

    // Cálculo dinámico de tamaños acumulados desde repo_db
    uint64_t total_download_bytes = 0;
    uint64_t total_install_bytes = 0;

    for (const auto& pkg : install_order) {
        auto it = repo_db.find(pkg);
        if (it != repo_db.end()) {
            total_download_bytes += it->second.size;
            total_install_bytes += it->second.installed_size;
        }
    }

    CLI_APK::printTransactionSummary(targets, deps_pkgs, total_download_bytes, total_install_bytes);

    if (!CLI_APK::confirm("¿Desea proceder con la instalación?")) {
        CLI_APK::printWarning("Operación cancelada por el usuario.");
        return;
    }

    fs::path installed_dir = fs::path(root_dir) / "var/lib/roger-apk/installed";
    fs::path cache_dir = "/root/tango-apk/pkgs";
    fs::path extra_cache_dir = "/root/tango-apk/pkgs-extra";

    fs::create_directories(installed_dir);
    fs::create_directories(cache_dir);

    // Mirrors apuntando a la rama edge coincidente con el APKINDEX de junio de 2026
    const std::string mirror_main = "https://dl-cdn.alpinelinux.org/alpine/edge/main/x86_64/";
    const std::string mirror_comm = "https://dl-cdn.alpinelinux.org/alpine/edge/community/x86_64/";

    for (size_t i = 0; i < install_order.size(); ++i) {
        const auto& pkg = install_order[i];
        CLI_APK::showProgressBar(i + 1, install_order.size(), "Instalando " + pkg);

        std::string version = repo_db[pkg].version;
        std::string filename = pkg + "-" + version + ".apk";

        fs::path apk_path = cache_dir / filename;

        // 1. Verificar si existe localmente (en pkgs/ o pkgs-extra/)
        if (!fs::exists(apk_path) && fs::exists(extra_cache_dir / filename)) {
            apk_path = extra_cache_dir / filename;
        }

        // 2. Si no existe en ningún directorio local, se descarga mediante curl
        if (!fs::exists(apk_path)) {
            std::string fetch_cmd = "curl -s -f -L " + mirror_main + filename + " -o " + apk_path.string() +
                                    " || curl -s -f -L " + mirror_comm + filename + " -o " + apk_path.string();
            int res = std::system(fetch_cmd.c_str());

            if (res != 0 || !fs::exists(apk_path)) {
                CLI_APK::printError("No se pudo descargar el paquete desde los mirrors: " + filename);
                return;
            }
        }

        // 3. Extraer el paquete .apk en la raíz del sistema
        std::string extract_cmd = "tar -xzf " + apk_path.string() + " -C " + root_dir + " 2>/dev/null";
        std::system(extract_cmd.c_str());

        // 4. Guardar metadatos de instalación
        std::ofstream meta(installed_dir / (pkg + ".meta"));
        meta << "pkgname: " << pkg << "\n";
        meta << "version: " << version << "\n";
        meta << "explicit: " << (std::find(targets.begin(), targets.end(), pkg) != targets.end() ? "1" : "0") << "\n";
        meta.close();
    }

    CLI_APK::printSuccess("Instalación completada con éxito.");
}

} // namespace RogerAPK
