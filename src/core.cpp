#include "psnn/core.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

namespace psnn {
namespace {
const char* const kProfilesFile = "profiles.psnn";
const char* const kActiveFile = "active.psnn";
const char* const kProfilesHeader = "PSNN_PROFILES_V1";
const char* const kActiveHeader = "PSNN_ACTIVE_V1";
const size_t kMaxProfiles = 64;
const size_t kMaxFileSize = 32768;

bool valid_root(const std::string& path) {
    if (path.empty() || path[0] != '/' || path == "/" || path.size() > 512) return false;
    if (path.find('\n') != std::string::npos || path.find('\r') != std::string::npos) return false;
    std::istringstream parts(path.substr(1));
    std::string segment;
    while (std::getline(parts, segment, '/')) {
        if (segment.empty() || segment == "." || segment == "..") return false;
    }
    return path.back() != '/';
}

bool valid_name(const std::string& s) {
    if (s.empty() || s.size() > 24) return false;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') || c == ' ' || c == '_' ||
                  c == '-' || c == '.';
        if (!ok) return false;
    }
    return true;
}

bool valid_id(const std::string& s) {
    if (s.size() != 14 || s.substr(0, 6) != "LOCAL-") return false;
    for (size_t i = 6; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

std::string path_join(const std::string& root, const char* name) {
    return root + "/" + name;
}

std::string os_error(const std::string& detail) {
    return detail + ": " + std::strerror(errno);
}

bool safe_directory(const std::string& path, std::string& error) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        error = os_error("cannot inspect directory " + path);
        return false;
    }
    if (!S_ISDIR(st.st_mode)) {
        error = "path is not a directory (or is a symlink): " + path;
        return false;
    }
    return true;
}

bool ensure_directory_tree(const std::string& path, std::string& error) {
    if (!valid_root(path)) { error = "root must be a safe absolute non-root path"; return false; }
    size_t start = 1;
    while (true) {
        size_t next = path.find('/', start);
        std::string part = path.substr(0, next);
        if (mkdir(part.c_str(), 0700) != 0 && errno != EEXIST) {
            error = os_error("mkdir " + part);
            return false;
        }
        if (!safe_directory(part, error)) return false;
        if (next == std::string::npos) break;
        start = next + 1;
    }
    return true;
}

bool read_bounded_file(const std::string& path, std::string& data, std::string& error) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) { error = os_error("cannot stat " + path); return false; }
    if (!S_ISREG(st.st_mode) || st.st_size < 0 || static_cast<size_t>(st.st_size) > kMaxFileSize) {
        error = "invalid or oversized data file: " + path;
        return false;
    }
    std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
    if (!f) { error = "cannot open " + path; return false; }
    std::ostringstream s;
    s << f.rdbuf();
    data = s.str();
    if (!f.eof() && f.fail()) { error = "cannot read " + path; return false; }
    if (data.size() > kMaxFileSize) { error = "file changed during read: " + path; return false; }
    return true;
}

// Atomic same-directory replacement: failed writes must not truncate old data.
bool atomic_write(const std::string& dest, const std::string& data, std::string& error) {
    if (data.size() > kMaxFileSize) { error = "data exceeds maximum size"; return false; }
    std::string pattern = dest + ".tmp.XXXXXX";
    std::vector<char> tmp(pattern.begin(), pattern.end());
    tmp.push_back('\0');
    int fd = mkstemp(&tmp[0]);
    if (fd < 0) { error = os_error("cannot create temporary file"); return false; }
    const std::string temporary(&tmp[0]);
    bool ok = true;
    if (fchmod(fd, 0600) != 0) { error = os_error("chmod failed"); ok = false; }
    size_t offset = 0;
    while (ok && offset < data.size()) {
        ssize_t n = write(fd, data.data() + offset, data.size() - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { error = os_error("write failed"); ok = false; break; }
        offset += static_cast<size_t>(n);
    }
    if (ok && fsync(fd) != 0) { error = os_error("fsync failed"); ok = false; }
    if (close(fd) != 0 && ok) { error = os_error("close failed"); ok = false; }
    if (ok && rename(temporary.c_str(), dest.c_str()) != 0) {
        error = os_error("atomic rename failed"); ok = false;
    }
    if (!ok) unlink(temporary.c_str());
    return ok;
}

bool directory_exists(const std::string& root) {
    struct stat st;
    return lstat(root.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string escape_json(const std::string& s) {
    std::ostringstream o;
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '"' || c == '\\') { o << '\\' << static_cast<char>(c); }
        else if (c < 0x20) {
            o << "\\u00" << hex[c >> 4] << hex[c & 15];
        } else o << static_cast<char>(c);
    }
    return o.str();
}
} // namespace

OfflineCore::OfflineCore(std::string root) : root_(root) {}

bool OfflineCore::initialize(std::string& error) {
    if (!ensure_directory_tree(root_, error)) return false;
    std::string content;
    if (access(path_join(root_, kProfilesFile).c_str(), F_OK) != 0) {
        if (errno != ENOENT) { error = os_error("cannot inspect profile file"); return false; }
        if (!atomic_write(path_join(root_, kProfilesFile), std::string(kProfilesHeader) + "\n", error)) return false;
    }
    // Do not replace a corrupt existing file during initialization.
    return load(error);
}

bool OfflineCore::load(std::string& error) {
    initialized_ = false;
    session_active_ = false;
    session_profile_id_.clear();
    profiles_.clear();
    active_profile_id_.clear();
    if (!valid_root(root_)) { error = "invalid root path"; return false; }
    if (!safe_directory(root_, error)) return false;
    std::string data;
    if (!read_bounded_file(path_join(root_, kProfilesFile), data, error)) return false;
    std::istringstream in(data);
    std::string line;
    if (!std::getline(in, line) || line != kProfilesHeader) {
        error = "unsupported or corrupt profiles file header"; return false;
    }
    while (std::getline(in, line)) {
        if (line.empty()) { error = "unexpected empty profile record"; profiles_.clear(); return false; }
        size_t sep = line.find('|');
        if (sep == std::string::npos || line.find('|', sep + 1) != std::string::npos) {
            error = "malformed profile record"; profiles_.clear(); return false;
        }
        Profile p = {line.substr(0, sep), line.substr(sep + 1)};
        if (!valid_id(p.id) || !valid_name(p.name) || profiles_.size() >= kMaxProfiles) {
            error = "invalid profile record or profile limit exceeded"; profiles_.clear(); return false;
        }
        for (size_t i = 0; i < profiles_.size(); ++i) {
            if (profiles_[i].id == p.id) { error = "duplicate profile ID"; profiles_.clear(); return false; }
        }
        profiles_.push_back(p);
    }
    const std::string active_path = path_join(root_, kActiveFile);
    if (access(active_path.c_str(), F_OK) == 0) {
        if (!read_bounded_file(active_path, data, error)) return false;
        std::istringstream active(data);
        if (!std::getline(active, line) || line != kActiveHeader || !std::getline(active, active_profile_id_)) {
            error = "corrupt active profile file"; return false;
        }
        if (std::getline(active, line) || !valid_id(active_profile_id_)) {
            error = "invalid active profile file"; return false;
        }
        bool found = false;
        for (size_t i = 0; i < profiles_.size(); ++i) {
            if (profiles_[i].id == active_profile_id_) { found = true; break; }
        }
        if (!found) { error = "active profile ID not found"; return false; }
    } else if (errno != ENOENT) {
        error = os_error("cannot inspect active profile file"); return false;
    }
    initialized_ = true;
    return true;
}

bool OfflineCore::save_profiles(std::string& error) const {
    std::ostringstream data;
    data << kProfilesHeader << "\n";
    for (size_t i = 0; i < profiles_.size(); ++i) {
        data << profiles_[i].id << '|' << profiles_[i].name << '\n';
    }
    return atomic_write(path_join(root_, kProfilesFile), data.str(), error);
}

bool OfflineCore::save_active(std::string& error) const {
    return atomic_write(path_join(root_, kActiveFile), std::string(kActiveHeader) + "\n" + active_profile_id_ + "\n", error);
}

bool OfflineCore::add_profile(const std::string& name, Profile& out, std::string& error) {
    if (!initialized_) { error = "initialize or load PSNN first"; return false; }
    if (!valid_name(name)) { error = "name must be 1-24 ASCII letters, digits, spaces, '.', '-' or '_'"; return false; }
    if (profiles_.size() >= kMaxProfiles) { error = "64 profile limit reached"; return false; }
    unsigned long next = 1;
    for (size_t i = 0; i < profiles_.size(); ++i) {
        unsigned long id = std::strtoul(profiles_[i].id.c_str() + 6, NULL, 10);
        if (next <= id) next = id + 1;
    }
    if (next > 99999999UL) { error = "profile ID range exhausted"; return false; }
    char id_buf[32];
    std::snprintf(id_buf, sizeof(id_buf), "LOCAL-%08lu", next);
    out.id = id_buf;
    out.name = name;
    profiles_.push_back(out);
    if (!save_profiles(error)) { profiles_.pop_back(); return false; }
    return true;
}

bool OfflineCore::select_profile(const std::string& id, std::string& error) {
    if (!initialized_) { error = "initialize or load PSNN first"; return false; }
    if (session_active_) { error = "end local session before switching profile"; return false; }
    bool found = false;
    for (size_t i = 0; i < profiles_.size(); ++i) {
        if (profiles_[i].id == id) { found = true; break; }
    }
    if (!found) { error = "profile not found"; return false; }
    std::string previous = active_profile_id_;
    active_profile_id_ = id;
    if (!save_active(error)) { active_profile_id_ = previous; return false; }
    return true;
}

bool OfflineCore::current_profile(Profile& out) const {
    if (!initialized_) return false;
    for (size_t i = 0; i < profiles_.size(); ++i) {
        if (profiles_[i].id == active_profile_id_) { out = profiles_[i]; return true; }
    }
    return false;
}

bool OfflineCore::start_local_session(std::string& error) {
    if (!initialized_) { error = "initialize or load PSNN first"; return false; }
    if (session_active_) { error = "session already active"; return false; }
    Profile p;
    if (!current_profile(p)) { error = "select a local profile first"; return false; }
    session_profile_id_ = p.id;
    session_active_ = true;
    return true;
}

void OfflineCore::end_local_session() {
    session_active_ = false;
    session_profile_id_.clear();
}

std::vector<Check> OfflineCore::diagnose() const {
    std::vector<Check> checks;
    checks.push_back(Check{"core", initialized_ ? "PASS" : "FAIL", "PSNN 0.1.0 offline-only runtime; no Sony authentication"});
    checks.push_back(Check{"storage_root", directory_exists(root_) ? "PASS" : "FAIL", root_});
    if (initialized_) {
        Profile p;
        checks.push_back(Check{"profiles", "PASS", std::to_string(profiles_.size()) + " valid local profile(s)"});
        checks.push_back(Check{"active_profile", current_profile(p) ? "PASS" : "INFO", current_profile(p) ? p.id : "none selected"});
        std::string error;
        std::string pattern = path_join(root_, ".diag.XXXXXX");
        std::vector<char> temp(pattern.begin(), pattern.end());
        temp.push_back('\0');
        int fd = mkstemp(&temp[0]);
        if (fd >= 0) {
            const char probe[] = "PSNN_OFFLINE_DIAG_V1";
            bool ok = write(fd, probe, sizeof(probe)) == static_cast<ssize_t>(sizeof(probe));
            if (ok) ok = fsync(fd) == 0;
            if (ok) ok = lseek(fd, 0, SEEK_SET) == 0;
            char got[sizeof(probe)] = {0};
            if (ok) ok = read(fd, got, sizeof(got)) == static_cast<ssize_t>(sizeof(got));
            if (ok) ok = std::memcmp(got, probe, sizeof(probe)) == 0;
            if (close(fd) != 0) ok = false;
            if (unlink(&temp[0]) != 0) ok = false;
            checks.push_back(Check{"storage_write_probe", ok ? "PASS" : "FAIL", "explicit temporary write/read/verify/fsync/delete probe"});
        } else {
            checks.push_back(Check{"storage_write_probe", "FAIL", os_error("cannot create diagnostic test file")});
        }
    } else {
        checks.push_back(Check{"profiles", "FAIL", "load/init PSNN before diagnostics"});
    }
    checks.push_back(Check{"local_session", "INFO", session_active_ ? "local in-memory session active" : "no local session active"});
    checks.push_back(Check{"sony_psn_auth", "INFO", "not implemented; never impersonated"});
    checks.push_back(Check{"network", "INFO", "disabled; no sockets or remote services"});
    checks.push_back(Check{"game_adapters", "INFO", "contract v1 defined; no hooks or adapters loaded"});
    checks.push_back(Check{"firmware", "WARN", "PS4 11.02 and 13.52 hardware tests not run"});
    return checks;
}

std::string diagnostic_json(const std::vector<Check>& checks) {
    std::ostringstream out;
    out << "{\"schema\":\"psnn.diagnostics.v1\",\"version\":\"" << kVersion << "\",\"checks\":[";
    for (size_t i = 0; i < checks.size(); ++i) {
        if (i) out << ',';
        out << "{\"name\":\"" << escape_json(checks[i].name)
            << "\",\"status\":\"" << escape_json(checks[i].status)
            << "\",\"detail\":\"" << escape_json(checks[i].detail) << "\"}";
    }
    out << "]}";
    return out.str();
}

} // namespace psnn
