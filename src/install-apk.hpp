#ifndef INSTALL_APK_HPP
#define INSTALL_APK_HPP

#include <string>
#include <vector>

namespace RogerAPK {

void cmd_install_apk(const std::string& root_dir, const std::vector<std::string>& targets);

} // namespace RogerAPK

#endif // INSTALL_APK_HPP
