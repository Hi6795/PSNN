#include "psnn/core.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

namespace {
int assertions = 0;
void require(bool condition, const std::string& msg) {
    ++assertions;
    if (!condition) throw std::runtime_error(msg);
}

void clean(const std::string& root) {
    unlink((root + "/profiles.psnn").c_str());
    unlink((root + "/active.psnn").c_str());
    rmdir(root.c_str());
}

void test_core() {
    char folder[] = "/tmp/psnn-core-test-XXXXXX";
    char* temp = mkdtemp(folder);
    require(temp != NULL, "mkdtemp");
    std::string root = temp;
    std::string error;
    try {
        psnn::OfflineCore core(root);
        require(!core.start_local_session(error), "session without load rejected");
        error.clear();
        require(core.initialize(error), "initialize: " + error);
        require(core.profiles().empty(), "starts with no identities");
        psnn::Profile invalid;
        require(!core.add_profile("../../bad", invalid, error), "invalid name rejected");
        error.clear();
        psnn::Profile alice, bob;
        require(core.add_profile("Alice", alice, error), "create Alice: " + error);
        require(core.add_profile("Bob_2", bob, error), "create Bob: " + error);
        require(alice.id == "LOCAL-00000001", "first stable profile id");
        require(bob.id == "LOCAL-00000002", "second stable profile id");
        require(!core.select_profile("LOCAL-99999999", error), "unknown identity rejected");
        require(core.select_profile(alice.id, error), "select Alice: " + error);
        psnn::Profile current;
        require(core.current_profile(current) && current.name == "Alice", "current identity");
        require(core.start_local_session(error), "local session begins");
        require(core.session_active(), "session active");
        require(!core.start_local_session(error), "duplicate session rejected");
        require(!core.select_profile(bob.id, error), "switching during session rejected");
        core.end_local_session();
        require(!core.session_active(), "session ended");
        require(core.select_profile(bob.id, error), "select Bob after session");
        std::vector<psnn::Check> checks = core.diagnose();
        require(checks.size() >= 8, "diagnostics available");
        bool storage_probe_ok = false;
        for (size_t i = 0; i < checks.size(); ++i) {
            if (checks[i].name == "storage_write_probe" && checks[i].status == "PASS") storage_probe_ok = true;
        }
        require(storage_probe_ok, "storage probe success");
        require(psnn::diagnostic_json(checks).find("\"schema\":\"psnn.diagnostics.v1\"") != std::string::npos,
                "diagnostics JSON schema");
        psnn::OfflineCore reloaded(root);
        require(reloaded.load(error), "reload persisted data: " + error);
        require(reloaded.profiles().size() == 2, "profiles persisted");
        require(reloaded.current_profile(current) && current.id == bob.id, "active persisted");
        require(!reloaded.session_active(), "local sessions never persist");
        require(reloaded.initialize(error), "repeat init must preserve data");
        require(reloaded.profiles().size() == 2, "init does not reset data");
        { // Existing corrupt file must not be silently overwritten.
            std::ofstream f((root + "/profiles.psnn").c_str(), std::ios::binary | std::ios::trunc);
            f << "NOT_A_PSNN_FILE\n";
        }
        require(!reloaded.load(error), "corrupt file rejected");
        require(!reloaded.initialize(error), "init refuses to overwrite corrupt file");
        std::ifstream f((root + "/profiles.psnn").c_str());
        std::string header;
        std::getline(f, header);
        require(header == "NOT_A_PSNN_FILE", "corrupt file preserved for recovery");
    } catch (...) {
        clean(root);
        throw;
    }
    clean(root);
}

void test_root_validation() {
    std::string e;
    psnn::OfflineCore root("/");
    require(!root.initialize(e), "cannot use filesystem root");
    psnn::OfflineCore traversal("/tmp/../unsafe");
    require(!traversal.initialize(e), "cannot traverse parent");
    psnn::OfflineCore relative("tmp/psnn");
    require(!relative.initialize(e), "must use absolute path");
}
}

int main() {
    try {
        test_core();
        test_root_validation();
        std::cout << "PASS " << assertions << " assertions (offline core)\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAIL after " << assertions << " assertions: " << ex.what() << "\n";
        return 1;
    }
}
