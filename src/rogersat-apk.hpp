#ifndef ROGER_SAT_APK_HPP
#define ROGER_SAT_APK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstdint>
#include <minisat/core/Solver.h>

namespace RogerAPK {

struct PackageSpec {
    std::string name;
    std::string version;
    std::string url;
    std::string sha256;
    uint64_t size = 0;           // Tamaño de descarga (campo S:)
    uint64_t installed_size = 0; // Tamaño instalado en disco (campo I:)
    std::vector<std::string> depends;
    bool explicit_installed = false;
};

class DependencyResolverSAT {
private:
    std::map<std::string, PackageSpec> db;

    std::string cleanDepName(const std::string& raw_dep) const {
        std::string dep = raw_dep;
        if (dep.rfind("so:", 0) == 0 || dep.rfind("cmd:", 0) == 0 || dep.rfind("pc:", 0) == 0) {
            return "";
        }
        size_t op_pos = dep.find_first_of("=<>-~");
        if (op_pos != std::string::npos) {
            dep = dep.substr(0, op_pos);
        }
        dep.erase(0, dep.find_first_not_of(" \t"));
        dep.erase(dep.find_last_not_of(" \t\r\n") + 1);
        return dep;
    }

public:
    void addPackage(const PackageSpec& pkg) {
        db[pkg.name] = pkg;
    }

    // Identifica paquetes absolutamente esenciales para la supervivencia del sistema/runtime
    bool isVitalPackage(const std::string& name) const {
        // Chequeo de librerías esenciales y herramientas base del gestor
        if (name == "musl" || name == "libc" || name == "glibc" || 
            name == "tar" || name == "zstd" || name == "curl" || name == "apk-tools") {
            return true;
        }
        return false;
    }

    // Calcula de forma dinámica qué paquetes instalados se romperían si se elimina 'target'
    std::vector<std::string> findReverseDependencies(const std::string& target, const std::set<std::string>& installed_pkgs) {
        std::vector<std::string> affected;
        for (const auto& pkg_name : installed_pkgs) {
            if (pkg_name == target) continue;
            auto it = db.find(pkg_name);
            if (it != db.end()) {
                for (const auto& raw_dep : it->second.depends) {
                    if (cleanDepName(raw_dep) == target) {
                        affected.push_back(pkg_name);
                        break;
                    }
                }
            }
        }
        return affected;
    }

    bool resolveInstall(const std::string& target, std::vector<std::string>& out_install_list) {
        if (db.find(target) == db.end()) {
            std::cerr << "[ERROR] El paquete APK '" << target << "' no existe en el índice.\n";
            return false;
        }

        Minisat::Solver solver;
        std::map<std::string, Minisat::Var> pkg_to_var;

        for (const auto& [name, pkg] : db) {
            pkg_to_var[name] = solver.newVar();
        }

        for (const auto& [name, pkg] : db) {
            Minisat::Var p_var = pkg_to_var[name];
            for (const auto& raw_dep : pkg.depends) {
                std::string dep = cleanDepName(raw_dep);
                if (dep.empty() || dep == name) continue;

                if (pkg_to_var.find(dep) == pkg_to_var.end()) {
                    continue; 
                }

                Minisat::Var dep_var = pkg_to_var[dep];
                Minisat::vec<Minisat::Lit> clause;
                clause.push(~Minisat::mkLit(p_var));
                clause.push(Minisat::mkLit(dep_var));
                solver.addClause_(clause);
            }
        }

        solver.addClause(Minisat::mkLit(pkg_to_var[target]));

        if (solver.solve()) {
            out_install_list.clear();
            for (const auto& [name, var] : pkg_to_var) {
                if (solver.modelValue(var) == Minisat::l_True) {
                    out_install_list.push_back(name);
                }
            }
            return true;
        }
        return false;
    }
};

} // namespace RogerAPK

#endif // ROGER_SAT_APK_HPP
