#pragma once

#include <string>
#include <vector>

namespace psnn {

static const int kDataVersion = 1;
static const int kAdapterAbi = 1;
static const char* const kVersion = "0.1.0";

struct Profile {
    std::string id;
    std::string name;
};

struct Check {
    std::string name;
    std::string status; // PASS, INFO, WARN, FAIL
    std::string detail;
};

class OfflineCore {
public:
    explicit OfflineCore(std::string root);

    // Explicit initialization only. No background workers or network requests.
    bool initialize(std::string& error);
    bool load(std::string& error);
    bool add_profile(const std::string& name, Profile& out, std::string& error);
    bool select_profile(const std::string& id, std::string& error);
    bool current_profile(Profile& out) const;
    const std::vector<Profile>& profiles() const { return profiles_; }

    // In-memory local demonstration. Not Sony NP/PSN authentication.
    bool start_local_session(std::string& error);
    void end_local_session();
    bool session_active() const { return session_active_; }
    std::string session_profile_id() const { return session_profile_id_; }

    // Diagnostic probe is opt-in and removes its own test file.
    std::vector<Check> diagnose() const;
    const std::string& root() const { return root_; }

private:
    std::string root_;
    std::vector<Profile> profiles_;
    std::string active_profile_id_;
    bool initialized_ = false;
    bool session_active_ = false;
    std::string session_profile_id_;

    bool save_profiles(std::string& error) const;
    bool save_active(std::string& error) const;
};

std::string diagnostic_json(const std::vector<Check>& checks);

} // namespace psnn
