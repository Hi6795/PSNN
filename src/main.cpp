#include "psnn/core.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {
void help() {
    std::cout << "PSNN 0.1.0 | Offline Core (NOT PlayStation Network)\n"
              << "Usage: psnn [--root ABSOLUTE_PATH] COMMAND\n\n"
              << "  init                       Create local storage explicitly\n"
              << "  profile add NAME           Create an offline local profile\n"
              << "  profile list               List local profiles\n"
              << "  profile select LOCAL-ID    Select local profile\n"
              << "  profile current            Display selected profile\n"
              << "  session demo               Start/end ephemeral LOCAL-only session\n"
              << "  diag [--json]              Diagnostics incl. temporary file probe\n"
              << "Default root on PS4: /data/PSNN\n"
              << "This program never connects to Sony or creates a real PSN session.\n";
}

int failure(const std::string& e) {
    std::cerr << "PSNN_ERROR: " << e << "\n";
    return 1;
}
}

int main(int argc, char* argv[]) {
    std::string root = "/data/PSNN";
    int at = 1;
    if (argc > 1 && std::string(argv[1]) == "--root") {
        if (argc < 4) { help(); return 2; }
        root = argv[2];
        at = 3;
    }
    if (argc <= at || std::string(argv[at]) == "--help") { help(); return argc <= at ? 2 : 0; }
    std::string command = argv[at++];
    psnn::OfflineCore core(root);
    std::string error;
    if (command == "init") {
        if (argc != at) return failure("init takes no arguments");
        if (!core.initialize(error)) return failure(error);
        std::cout << "PSNN_READY offline_core=1 root=" << root << "\n";
        return 0;
    }
    if (!core.load(error)) return failure(error + " (run 'init' if new)");

    if (command == "profile") {
        if (argc <= at) { help(); return 2; }
        std::string action = argv[at++];
        if (action == "list" && argc == at) {
            for (size_t i = 0; i < core.profiles().size(); ++i) {
                psnn::Profile current;
                bool selected = core.current_profile(current) && current.id == core.profiles()[i].id;
                std::cout << (selected ? "* " : "  ") << core.profiles()[i].id << "  " << core.profiles()[i].name << "\n";
            }
            return 0;
        }
        if (action == "current" && argc == at) {
            psnn::Profile p;
            if (!core.current_profile(p)) return failure("no selected profile");
            std::cout << "PSNN_CURRENT " << p.id << " " << p.name << " (local only)\n";
            return 0;
        }
        if (action == "add" && argc > at) {
            std::string name = argv[at++];
            while (at < argc) { name += " "; name += argv[at++]; }
            psnn::Profile p;
            if (!core.add_profile(name, p, error)) return failure(error);
            std::cout << "PSNN_PROFILE_CREATED " << p.id << " " << p.name << "\n";
            return 0;
        }
        if (action == "select" && argc == at + 1) {
            if (!core.select_profile(argv[at], error)) return failure(error);
            std::cout << "PSNN_PROFILE_SELECTED " << argv[at] << "\n";
            return 0;
        }
        help(); return 2;
    }
    if (command == "session" && argc == at + 1 && std::string(argv[at]) == "demo") {
        if (!core.start_local_session(error)) return failure(error);
        std::cout << "PSNN_LOCAL_SESSION_OPEN profile=" << core.session_profile_id() << " sony_auth=0 network=0\n";
        core.end_local_session();
        std::cout << "PSNN_LOCAL_SESSION_CLOSED\n";
        return 0;
    }
    if (command == "diag") {
        bool json = (argc == at + 1 && std::string(argv[at]) == "--json");
        if (!json && argc != at) { help(); return 2; }
        std::vector<psnn::Check> checks = core.diagnose();
        if (json) std::cout << psnn::diagnostic_json(checks) << '\n';
        else {
            std::cout << "PSNN DIAGNOSTICS v" << psnn::kVersion << "\n";
            for (size_t i = 0; i < checks.size(); ++i) {
                std::cout << '[' << checks[i].status << "] " << checks[i].name << " :: " << checks[i].detail << "\n";
            }
        }
        for (size_t i = 0; i < checks.size(); ++i) {
            if (checks[i].status == "FAIL") return 1;
        }
        return 0;
    }
    help();
    return 2;
}
