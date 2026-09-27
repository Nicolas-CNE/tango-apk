#include "rogersat-apk.hpp"
#include "cli-apk.hpp"
#include "list-apk.hpp"
#include "sync-apk.hpp"
#include "install-apk.hpp"
#include "delete-apk.hpp"
#include "upgrade-apk.hpp"
#include "bomb-apk.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        CLI_APK::printInfo("Uso: roger-apk <comando> [opciones/paquetes]");
        CLI_APK::printInfo("Comandos disponibles: sync, update, install, delete, upgrade, bomb, list");
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    const std::string root_dir = "/";

    if (command == "sync") {
        RogerAPK::cmd_sync_apk(root_dir);
    } else if (command == "update") {
        RogerAPK::cmd_update_apk(root_dir);
    } else if (command == "install") {
        RogerAPK::cmd_install_apk(root_dir, args);
    } else if (command == "delete" || command == "remove") {
        RogerAPK::cmd_delete_apk(root_dir, args);
    } else if (command == "upgrade") {
        RogerAPK::cmd_upgrade_apk(root_dir);
    } else if (command == "bomb") {
        RogerAPK::cmd_bomb_apk(root_dir);
    } else if (command == "list") {
        cmd_list_apk(root_dir);
    } else {
        CLI_APK::printError("Comando desconocido: " + command);
        return 1;
    }

    return 0;
}
