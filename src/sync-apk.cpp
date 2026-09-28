#include "sync-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace RogerAPK {

static void parseAPKINDEXFile(const fs::path& filepath, std::ofstream& out_db) {
    if (!fs::exists(filepath)) {
        CLI_APK::printWarning("No se encontró el índice: " + filepath.string());
        return;
    }

    std::ifstream file(filepath);
    std::string line;
    std::string pkgname, version, depends, size = "0", installed_size = "0";

    auto write_entry = [&]() {
        if (!pkgname.empty()) {
            out_db << "pkgname: " << pkgname << "\n";
            out_db << "version: " << version << "\n";
            out_db << "size: " << size << "\n";
            out_db << "installed_size: " << installed_size << "\n";
            out_db << "depends: " << depends << "\n";
            out_db << "---\n";
        }
    };

    while (std::getline(file, line)) {
        if (line.empty()) {
            write_entry();
            pkgname.clear();
            version.clear();
            depends.clear();
            size = "0";
            installed_size = "0";
            continue;
        }

        if (line.rfind("P:", 0) == 0) pkgname = line.substr(2);
        else if (line.rfind("V:", 0) == 0) version = line.substr(2);
        else if (line.rfind("S:", 0) == 0) size = line.substr(2);
        else if (line.rfind("I:", 0) == 0) installed_size = line.substr(2);
        else if (line.rfind("D:", 0) == 0) depends = line.substr(2);
    }
    write_entry();
}

void cmd_sync_apk(const std::string& root_dir) {
    CLI_APK::printInfo("Descargando e indexando repositorios oficiales de Alpine...");

    fs::path db_dir = fs::path(root_dir) / "var/lib/roger-apk";
    fs::create_directories(db_dir);
    fs::path repo_index = db_dir / "repo_index";

    // Obtener los índices directamente vía HTTP
    std::system("curl -s -L https://dl-cdn.alpinelinux.org/alpine/edge/main/x86_64/APKINDEX.tar.gz -o /tmp/APKINDEX-main.tar.gz");
    std::system("tar -xzf /tmp/APKINDEX-main.tar.gz -C /tmp/ APKINDEX && mv /tmp/APKINDEX /tmp/APKINDEX-main");

    std::system("curl -s -L https://dl-cdn.alpinelinux.org/alpine/edge/community/x86_64/APKINDEX.tar.gz -o /tmp/APKINDEX-extra.tar.gz");
    std::system("tar -xzf /tmp/APKINDEX-extra.tar.gz -C /tmp/ APKINDEX && mv /tmp/APKINDEX /tmp/APKINDEX-extra");

    std::ofstream out_file(repo_index);
    if (!out_file.is_open()) {
        CLI_APK::printError("No se pudo escribir en el índice local.");
        return;
    }

    parseAPKINDEXFile("/tmp/APKINDEX-main", out_file);
    parseAPKINDEXFile("/tmp/APKINDEX-extra", out_file);

    out_file.close();

    // Limpiar temporales
    fs::remove("/tmp/APKINDEX-main.tar.gz");
    fs::remove("/tmp/APKINDEX-extra.tar.gz");
    fs::remove("/tmp/APKINDEX-main");
    fs::remove("/tmp/APKINDEX-extra");

    CLI_APK::printSuccess("Base de datos local sincronizada correctamente sin binarios en el repo.");
}

void cmd_update_apk(const std::string& root_dir) {
    cmd_sync_apk(root_dir);
}

} // namespace RogerAPK
