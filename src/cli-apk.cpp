#include "cli-apk.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace CLI_APK {

std::string formatSize(size_t bytes) {
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    double size = static_cast<double>(bytes);
    int unit_idx = 0;

    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
    return ss.str();
}

bool confirm(const std::string& prompt_msg) {
    std::cout << prompt_msg << " [S/n]: ";
    std::string response;
    if (!std::getline(std::cin, response)) return false;
    
    if (response.empty() || response == "s" || response == "S" || response == "si" || response == "SI" || response == "y" || response == "Y") {
        return true;
    }
    return false;
}

void showProgressBar(size_t current, size_t total, const std::string& prefix) {
    if (total == 0) return;
    const int bar_width = 30;
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(bar_width * progress);

    std::cout << "\r" << prefix << " [";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" << current << "/" << total << ")" << std::flush;
    if (current == total) {
        std::cout << std::endl;
    }
}

void printTransactionSummary(
    const std::vector<std::string>& explicit_pkgs,
    const std::vector<std::string>& deps_pkgs,
    size_t total_download_bytes,
    size_t total_install_bytes
) {
    std::cout << "\n=========================================\n";
    std::cout << "     RESUMEN DE TRANSACCIÓN (ROGER-APK)  \n";
    std::cout << "=========================================\n";

    if (!explicit_pkgs.empty()) {
        std::cout << "Paquetes solicitados (" << explicit_pkgs.size() << "):\n  ";
        for (const auto& pkg : explicit_pkgs) std::cout << pkg << " ";
        std::cout << "\n\n";
    }

    if (!deps_pkgs.empty()) {
        std::cout << "Dependencias APK requeridas (" << deps_pkgs.size() << "):\n  ";
        for (const auto& pkg : deps_pkgs) std::cout << pkg << " ";
        std::cout << "\n\n";
    }

    std::cout << "Paquetes totales:         " << (explicit_pkgs.size() + deps_pkgs.size()) << "\n";
    if (total_download_bytes > 0) {
        std::cout << "Descarga estimada:       " << formatSize(total_download_bytes) << "\n";
    }
    if (total_install_bytes > 0) {
        std::cout << "Espacio en disco extra:  " << formatSize(total_install_bytes) << "\n";
    }
    std::cout << "=========================================\n\n";
}

void printDeleteSummary(
    const std::vector<std::string>& explicit_pkgs,
    const std::vector<std::string>& orphan_deps_pkgs,
    size_t total_freed_bytes
) {
    std::cout << "\n=========================================\n";
    std::cout << "    ELIMINACIÓN DE PAQUETES (ROGER-APK)  \n";
    std::cout << "=========================================\n";

    if (!explicit_pkgs.empty()) {
        std::cout << "Paquetes a eliminar (" << explicit_pkgs.size() << "):\n  ";
        for (const auto& pkg : explicit_pkgs) std::cout << pkg << " ";
        std::cout << "\n\n";
    }

    if (!orphan_deps_pkgs.empty()) {
        std::cout << "Huérfanos a remover (" << orphan_deps_pkgs.size() << "):\n  ";
        for (const auto& pkg : orphan_deps_pkgs) std::cout << pkg << " ";
        std::cout << "\n\n";
    }

    std::cout << "Paquetes totales:           " << (explicit_pkgs.size() + orphan_deps_pkgs.size()) << "\n";
    std::cout << "Espacio a liberar:          " << formatSize(total_freed_bytes) << "\n";
    std::cout << "=========================================\n\n";
}

void printSuccess(const std::string& msg) {
    std::cout << "[ROGER-APK OK] " << msg << "\n";
}

void printError(const std::string& msg) {
    std::cerr << "[ROGER-APK ERROR] " << msg << "\n";
}

void printWarning(const std::string& msg) {
    std::cout << "[ROGER-APK ADVERTENCIA] " << msg << "\n";
}

void printInfo(const std::string& msg) {
    std::cout << "[ROGER-APK INFO] " << msg << "\n";
}

} // namespace CLI_APK
