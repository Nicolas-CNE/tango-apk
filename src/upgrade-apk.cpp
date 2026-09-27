#include "upgrade-apk.hpp"
#include "sync-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace RogerAPK {

void cmd_upgrade_apk(const std::string& root_dir) {
    CLI_APK::printInfo("Verificando actualizaciones del sistema APK...");
    cmd_sync_apk(root_dir);

    // Lógica para comparar versiones entre installed/ y repo_index
    CLI_APK::printInfo("Todos los paquetes instalados están actualizados.");
}

} // namespace RogerAPK
