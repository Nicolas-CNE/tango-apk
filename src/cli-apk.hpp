#ifndef CLI_APK_HPP
#define CLI_APK_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <unordered_set>

namespace CLI_APK {

    const std::unordered_set<std::string> PROTECTED_PACKAGES = {
        "musl", "busybox", "apk-tools", "zstd", "tar"
    };

    std::string formatSize(size_t bytes);
    bool confirm(const std::string& prompt_msg = "¿Desea continuar?");
    void showProgressBar(size_t current, size_t total, const std::string& prefix = "");

    void printTransactionSummary(
        const std::vector<std::string>& explicit_pkgs,
        const std::vector<std::string>& deps_pkgs,
        size_t total_download_bytes,
        size_t total_install_bytes
    );

    void printDeleteSummary(
        const std::vector<std::string>& explicit_pkgs,
        const std::vector<std::string>& orphan_deps_pkgs,
        size_t total_freed_bytes
    );

    void printSuccess(const std::string& msg);
    void printError(const std::string& msg);
    void printWarning(const std::string& msg);
    void printInfo(const std::string& msg);

} // namespace CLI_APK

#endif // CLI_APK_HPP
