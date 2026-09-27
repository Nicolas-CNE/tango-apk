#include "list-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <iomanip>

namespace fs = std::filesystem;

void cmd_list_apk(const std::string& root_dir) {
    // Apunta a la base de datos exclusiva de roger-apk
    fs::path db_path = fs::path(root_dir) / "var/lib/roger-apk/installed";

    if (!fs::exists(db_path) || fs::is_empty(db_path)) {
        CLI_APK::printInfo("No hay paquetes APK instalados en el sistema.");
        return;
    }

    std::map<std::string, std::string> installed_pkgs; // <nombre, versión>

    for (const auto& entry : fs::directory_iterator(db_path)) {
        if (entry.path().extension() == ".meta") {
            std::ifstream file(entry.path());
            std::string line;
            std::string pkg_name, pkg_version;

            while (std::getline(file, line)) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string key = line.substr(0, colon);
                    std::string val = line.substr(colon + 1);

                    val.erase(0, val.find_first_not_of(" \t"));
                    val.erase(val.find_last_not_of(" \t\r\n") + 1);

                    if (key == "pkgname") pkg_name = val;
                    else if (key == "version") pkg_version = val;
                }
            }

            if (!pkg_name.empty()) {
                installed_pkgs[pkg_name] = pkg_version;
            }
        }
    }

    if (installed_pkgs.empty()) {
        CLI_APK::printInfo("No se encontraron metadatos de paquetes APK.");
        return;
    }

    std::cout << "\nPaquetes APK instalados (" << installed_pkgs.size() << "):\n";
    std::cout << "----------------------------------------\n";
    for (const auto& [name, version] : installed_pkgs) {
        std::cout << " - " << std::left << std::setw(25) << name << " " << version << "\n";
    }
    std::cout << "----------------------------------------\n";
}
