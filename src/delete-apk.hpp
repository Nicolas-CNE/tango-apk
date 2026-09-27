#ifndef DELETE_APK_HPP
#define DELETE_APK_HPP

#include <string>
#include <vector>

namespace RogerAPK {

void cmd_delete_apk(const std::string& root_dir, const std::vector<std::string>& targets);

} // namespace RogerAPK

#endif // DELETE_APK_HPP
