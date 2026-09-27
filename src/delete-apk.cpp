#include "delete-apk.hpp"
#include "rogersat-apk.hpp"
#include "cli-apk.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace RogerAPK {

void cmd_delete_apk(const std::string& root_dir, const std::vector<std::string>& targets) {
    if (targets.empty()) {
        CLI_APK::printError("Debe especificar al menos un paquete para eliminar.");
        return;
    }

    DependencyResolverSAT resolver;
    for (const auto& target : targets) {
        if (resolver.isVitalPackage(target) || CLI_APK::PROTECTED_PACKAGES.count(target)) {
            CLI_APK::printError("El paquete '" + target + "' está protegido por el sistema base y no se puede eliminar.");
            return;
        }
    }

    CLI_APK::printDeleteSummary(targets, {}, 1024 * 512);

    if (!CLI_APK::confirm("¿Está seguro de que desea desinstalar estos paquetes?")) {
        CLI_APK::printWarning("Operación cancelada.");
        return;
    }

    fs::path installed_dir = fs::path(root_dir) / "var/lib/roger-apk/installed";

    for (const auto& pkg : targets) {
        fs::path meta_path = installed_dir / (pkg + ".meta");
        if (fs::exists(meta_path)) {
            fs::remove(meta_path);
            CLI_APK::printSuccess("Removido: " + pkg);
        } else {
            CLI_APK::printWarning("El paquete no estaba instalado: " + pkg);
        }
    }
}

} // namespace RogerAPK
