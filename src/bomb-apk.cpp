#include "bomb-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace RogerAPK {

void cmd_bomb_apk(const std::string& root_dir) {
    CLI_APK::printWarning("¡ADVERTENCIA! El comando BOMB eliminará TODOS los paquetes no protegidos.");
    
    if (!CLI_APK::confirm("¿Desea purgar completamente el entorno APK instalados?")) {
        CLI_APK::printInfo("Operación de purga abortada.");
        return;
    }

    fs::path db_path = fs::path(root_dir) / "var/lib/roger-apk/installed";
    if (fs::exists(db_path)) {
        for (const auto& entry : fs::directory_iterator(db_path)) {
            std::string stem = entry.path().stem().string();
            if (CLI_APK::PROTECTED_PACKAGES.count(stem) == 0) {
                fs::remove(entry.path());
                CLI_APK::printInfo("Purgado: " + stem);
            }
        }
    }
    CLI_APK::printSuccess("Entorno purgado con éxito.");
}

} // namespace RogerAPK
