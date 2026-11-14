#include "FWLSkunkCrafts.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <winreg.h>
#else
#include <dlfcn.h>
#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif
#endif

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "FlyWithLua.h"
#include "FlyWithLuaImGui.h"
#include "ImgWindow.h"
#include "imgui.h"
#include "contrib.h"

namespace flywithlua {
namespace scu {

namespace {

constexpr const char* kDepotReleaseURL =
    "https://sparker256.github.io/FlyWithLua/depot/release/FlyWithLua";
constexpr const char* kDepotBetaURL =
    "https://sparker256.github.io/FlyWithLua/depot/beta/FlyWithLua";
constexpr const char* kOrgDownloadURL =
    "https://forums.x-plane.org/index.php?/files/file/82888-flywithlua-ng-next-generation-plus-edition-for-x-plane-12-win-lin-mac/";
constexpr const char* kSkunkCraftsWebsiteURL =
    "https://updater.skunkcrafts.com";
constexpr const char* kPrefsFilePath =
    "./Resources/plugins/FlyWithLua/fwl_prefs.ini";

// ----------------------------------------------------------------------------
// String & SkunkCrafts Key|Value Helpers
// ----------------------------------------------------------------------------

std::string TrimWhitespace(const std::string& input) {
    size_t first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, last - first + 1);
}

std::string CfgValueFromText(const std::string& content, const char* key) {
    std::istringstream iss(content);
    std::string want = std::string(key) + "|";
    std::string line;
    while (std::getline(iss, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        if (line.compare(0, want.size(), want) == 0) {
            return TrimWhitespace(line.substr(want.size()));
        }
    }
    return "";
}

std::string CfgValueFromFile(const std::string& path, const char* key) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        return "";
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return CfgValueFromText(oss.str(), key);
}

bool PathExists(const std::string& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool EnsureEndsWithNewline(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        return false;
    }
    ifs.seekg(0, std::ios::end);
    std::streamoff len = ifs.tellg();
    if (len <= 0) {
        return true;
    }
    ifs.seekg(len - 1, std::ios::beg);
    char lastCh = '\0';
    ifs.get(lastCh);
    ifs.close();
    if (lastCh != '\n') {
        std::ofstream ofs(path, std::ios::out | std::ios::app | std::ios::binary);
        if (!ofs) {
            return false;
        }
        ofs.put('\n');
    }
    return true;
}

std::string JoinPath(const std::string& base, const std::string& leaf) {
    if (base.empty()) {
        return leaf;
    }
    char last = base.back();
    if (last == '/' || last == '\\') {
        return base + leaf;
    }
    return base + "/" + leaf;
}

bool IsRegularFileOrBundle(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return false;
    }
#if defined(__APPLE__)
    if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".app") == 0) {
        return std::filesystem::is_directory(path, ec);
    }
#endif
    return std::filesystem::is_regular_file(path, ec);
}

std::string GetEnvString(const char* name) {
    const char* val = std::getenv(name);
    return val ? std::string(val) : std::string();
}

#if !defined(_WIN32)
std::string QuotePosixSingle(const std::string& raw) {
    std::string out = "'";
    for (char ch : raw) {
        if (ch == '\'') {
            out += "'\\''";
        } else {
            out.push_back(ch);
        }
    }
    out.push_back('\'');
    return out;
}
#endif

// ----------------------------------------------------------------------------
// Cross-Platform SkunkCrafts Updater (v3 & v4) Detection and Launch
// ----------------------------------------------------------------------------
// Notes:
//   - Only an actual existing executable / .app bundle on disk counts as
//     installed (leftover prefs.cfg / updater_log.txt files are never treated
//     as proof that the app is still present).
//   - Distinguishes SCU v4.0+ from legacy v3.x standalone binaries and detects
//     the retired 2020 in-sim plugin (Resources/plugins/SkunkCraftsUpdater) so
//     we can gently nudge users to upgrade to SCU v4 at updater.skunkcrafts.com.

struct UpdaterAvailability {
    bool isInstalled = false;
    bool isV4OrNewer = false;
    bool hasDeprecatedPlugin = false;
    std::string launchTarget;
    std::string detectedVersion;
};

bool IsDirectoryPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

std::string DetectUpdaterAppVersion(const std::string& appPath, bool& outIsV4) {
    outIsV4 = false;
#if defined(__APPLE__)
    std::ifstream ifs(appPath + "/Contents/Info.plist", std::ios::in | std::ios::binary);
    if (ifs) {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        std::string plist = oss.str();
        size_t keyPos = plist.find("CFBundleShortVersionString");
        if (keyPos != std::string::npos) {
            size_t s1 = plist.find("<string>", keyPos);
            size_t s2 = (s1 != std::string::npos) ? plist.find("</string>", s1 + 8) : std::string::npos;
            if (s1 != std::string::npos && s2 != std::string::npos) {
                std::string ver = TrimWhitespace(plist.substr(s1 + 8, s2 - (s1 + 8)));
                if (!ver.empty()) {
                    outIsV4 = (std::atoi(ver.c_str()) >= 4);
                    return ver;
                }
            }
        }
    }
#else
    // Every SCU v4 binary on Windows and Linux embeds "SkunkCrafts Updater 4.<minor>.<patch>"
    FILE* fp = std::fopen(appPath.c_str(), "rb");
    if (fp) {
        constexpr const char* kPrefix = "SkunkCrafts Updater ";
        constexpr size_t kPrefixLen = sizeof("SkunkCrafts Updater ") - 1;
        const std::string needle = "SkunkCrafts Updater 4.";
        std::string carry;
        char chunk[65536];
        size_t n = 0;
        while ((n = std::fread(chunk, 1, sizeof(chunk), fp)) > 0) {
            std::string window = carry + std::string(chunk, n);
            size_t pos = window.find(needle);
            if (pos != std::string::npos) {
                std::fclose(fp);
                size_t verStart = pos + kPrefixLen;
                size_t verEnd = verStart;
                while (verEnd < window.size() &&
                       (std::isdigit(static_cast<unsigned char>(window[verEnd])) || window[verEnd] == '.')) {
                    ++verEnd;
                }
                (void)kPrefix;
                outIsV4 = true;
                return window.substr(verStart, verEnd - verStart);
            }
            carry = (window.size() > 64) ? window.substr(window.size() - 64) : window;
        }
        std::fclose(fp);
    }
#endif
    return "3.x";
}

#if defined(_WIN32)
std::string QueryWinRegString(HKEY rootKey, const char* subKey, const char* valueName, REGSAM wowFlags) {
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(rootKey, subKey, 0, KEY_READ | wowFlags, &hKey) != ERROR_SUCCESS || !hKey) {
        return "";
    }
    char buf[2048] = {};
    DWORD bufSize = static_cast<DWORD>(sizeof(buf) - 1);
    DWORD regType = 0;
    LSTATUS status = RegQueryValueExA(
        hKey,
        (valueName && valueName[0] != '\0') ? valueName : nullptr,
        nullptr,
        &regType,
        reinterpret_cast<LPBYTE>(buf),
        &bufSize);
    RegCloseKey(hKey);
    if (status != ERROR_SUCCESS || (regType != REG_SZ && regType != REG_EXPAND_SZ)) {
        return "";
    }
    buf[sizeof(buf) - 1] = '\0';
    return TrimWhitespace(buf);
}

std::string NormalizeWinExeCandidate(const std::string& raw) {
    std::string s = TrimWhitespace(raw);
    if (s.empty()) {
        return "";
    }
    if (s.front() == '"') {
        size_t endQuote = s.find('"', 1);
        if (endQuote != std::string::npos) {
            s = s.substr(1, endQuote - 1);
        }
    } else {
        size_t commaPos = s.rfind(',');
        if (commaPos != std::string::npos && commaPos > 4) {
            s = TrimWhitespace(s.substr(0, commaPos));
        }
    }
    if (IsRegularFileOrBundle(s)) {
        return s;
    }
    if (IsDirectoryPath(s)) {
        std::string joined = JoinPath(s, "SkunkcraftsUpdater.exe");
        if (IsRegularFileOrBundle(joined)) {
            return joined;
        }
    }
    return "";
}
#endif

#if defined(__APPLE__)
std::string FindMacBundleViaLaunchServices() {
    using LSCopyURLsFn = CFArrayRef (*)(CFStringRef, CFErrorRef*);
    auto fn = reinterpret_cast<LSCopyURLsFn>(
        dlsym(RTLD_DEFAULT, "LSCopyApplicationURLsForBundleIdentifier"));
    if (!fn) {
        return "";
    }
    CFArrayRef urls = fn(CFSTR("com.skunkcrafts.updater"), nullptr);
    if (!urls) {
        return "";
    }
    std::string bestMatch;
    CFIndex count = CFArrayGetCount(urls);
    for (CFIndex i = 0; i < count; ++i) {
        auto url = static_cast<CFURLRef>(CFArrayGetValueAtIndex(urls, i));
        if (!url) {
            continue;
        }
        UInt8 pathBuf[2048] = {};
        if (!CFURLGetFileSystemRepresentation(url, true, pathBuf, sizeof(pathBuf) - 1)) {
            continue;
        }
        std::string candidate(reinterpret_cast<const char*>(pathBuf));
        // Skip quarantined Downloads copies (SCU v4.2.9+ refuses to self-update from Downloads) and Trash:
        if (candidate.find("/Downloads/") != std::string::npos ||
            candidate.find("/.Trash/") != std::string::npos) {
            continue;
        }
        if (IsRegularFileOrBundle(candidate)) {
            bestMatch = candidate;
            break;
        }
    }
    CFRelease(urls);
    return bestMatch;
}
#endif

#if defined(__linux__)
std::string ExtractExecFromDesktopFile(const std::string& desktopPath) {
    std::ifstream ifs(desktopPath);
    if (!ifs) {
        return "";
    }
    std::string line;
    while (std::getline(ifs, line)) {
        line = TrimWhitespace(line);
        if (line.rfind("Exec=", 0) != 0) {
            continue;
        }
        std::string cmd = TrimWhitespace(line.substr(5));
        if (cmd.empty()) {
            continue;
        }
        std::string binPath;
        if (cmd.front() == '"') {
            size_t endQuote = cmd.find('"', 1);
            if (endQuote != std::string::npos) {
                binPath = cmd.substr(1, endQuote - 1);
            }
        } else {
            size_t spacePos = cmd.find(' ');
            binPath = (spacePos == std::string::npos) ? cmd : cmd.substr(0, spacePos);
        }
        if (IsRegularFileOrBundle(binPath)) {
            return binPath;
        }
    }
    return "";
}
#endif

UpdaterAvailability DetectSkunkCraftsUpdater() {
    UpdaterAvailability out;
    const std::string simRoot = !systemDir.empty() ? systemDir : std::string(".");
    out.hasDeprecatedPlugin =
        IsDirectoryPath(JoinPath(simRoot, "Resources/plugins/SkunkCraftsUpdater")) ||
        IsDirectoryPath(JoinPath(simRoot, "Resources/plugins/skunkcrafts_updater"));

    std::vector<std::string> candidates;
#if defined(_WIN32)
    const std::string localAppData = GetEnvString("LOCALAPPDATA");
    const std::string progFiles = GetEnvString("ProgramFiles");
    const std::string progFilesX86 = GetEnvString("ProgramFiles(x86)");

    if (!localAppData.empty()) {
        candidates.push_back(JoinPath(localAppData, "Programs\\SkunkCrafts Updater\\SkunkcraftsUpdater.exe"));
        candidates.push_back(JoinPath(localAppData, "Programs\\SkunkcraftsUpdater\\SkunkcraftsUpdater.exe"));
        candidates.push_back(JoinPath(localAppData, "SkunkCrafts\\Updater\\SkunkcraftsUpdater.exe"));
    }
    if (!progFiles.empty()) {
        candidates.push_back(JoinPath(progFiles, "SkunkCrafts Updater\\SkunkcraftsUpdater.exe"));
        candidates.push_back(JoinPath(progFiles, "SkunkcraftsUpdater\\SkunkcraftsUpdater.exe"));
    }
    if (!progFilesX86.empty()) {
        candidates.push_back(JoinPath(progFilesX86, "SkunkCrafts Updater\\SkunkcraftsUpdater.exe"));
        candidates.push_back(JoinPath(progFilesX86, "SkunkcraftsUpdater\\SkunkcraftsUpdater.exe"));
    }

    const HKEY roots[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    const REGSAM views[] = {0, KEY_WOW64_64KEY, KEY_WOW64_32KEY};
    const char* appPathKeys[] = {
        "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\SkunkcraftsUpdater.exe"
    };
    const char* uninstallKeys[] = {
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SkunkCrafts Updater",
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SkunkcraftsUpdater"
    };

    for (HKEY root : roots) {
        for (REGSAM view : views) {
            for (const char* k : appPathKeys) {
                std::string hit = NormalizeWinExeCandidate(QueryWinRegString(root, k, "", view));
                if (!hit.empty()) {
                    candidates.push_back(hit);
                }
            }
            for (const char* k : uninstallKeys) {
                for (const char* valName : {"InstallLocation", "DisplayIcon"}) {
                    std::string hit = NormalizeWinExeCandidate(QueryWinRegString(root, k, valName, view));
                    if (!hit.empty()) {
                        candidates.push_back(hit);
                    }
                }
            }
        }
    }

    candidates.push_back(JoinPath(simRoot, "SkunkcraftsUpdater.exe"));
    if (simRoot != ".") {
        candidates.push_back(".\\SkunkcraftsUpdater.exe");
    }
#elif defined(__APPLE__)
    const std::string home = GetEnvString("HOME");
    candidates.push_back("/Applications/SkunkcraftsUpdater.app");
    if (!home.empty()) {
        candidates.push_back(JoinPath(home, "Applications/SkunkcraftsUpdater.app"));
    }
    candidates.push_back(JoinPath(simRoot, "SkunkcraftsUpdater.app"));
    if (simRoot != ".") {
        candidates.push_back("./SkunkcraftsUpdater.app");
    }
    std::string lsBundle = FindMacBundleViaLaunchServices();
    if (!lsBundle.empty()) {
        candidates.push_back(lsBundle);
    }
#else
    const std::string home = GetEnvString("HOME");
    const std::string xdgData = GetEnvString("XDG_DATA_HOME");

    if (!home.empty()) {
        candidates.push_back(JoinPath(home, ".local/bin/SkunkcraftsUpdater_lin"));
        candidates.push_back(JoinPath(home, ".local/bin/SkunkcraftsUpdater-x86_64.AppImage"));
        candidates.push_back(JoinPath(home, "Applications/SkunkcraftsUpdater-x86_64.AppImage"));
        candidates.push_back(JoinPath(home, "Applications/SkunkcraftsUpdater.AppImage"));
        candidates.push_back(JoinPath(home, "Applications/SkunkcraftsUpdater_lin"));
        candidates.push_back(JoinPath(home, "bin/SkunkcraftsUpdater_lin"));
    }
    candidates.push_back("/usr/local/bin/SkunkcraftsUpdater_lin");
    candidates.push_back("/usr/bin/SkunkcraftsUpdater_lin");

    std::vector<std::string> desktopDirs;
    if (!xdgData.empty()) {
        desktopDirs.push_back(JoinPath(xdgData, "applications"));
    }
    if (!home.empty()) {
        desktopDirs.push_back(JoinPath(home, ".local/share/applications"));
    }
    desktopDirs.push_back("/usr/local/share/applications");
    desktopDirs.push_back("/usr/share/applications");
    for (const auto& dir : desktopDirs) {
        for (const char* df : {"SkunkcraftsUpdater.desktop", "skunkcraftsupdater.desktop"}) {
            std::string execBin = ExtractExecFromDesktopFile(JoinPath(dir, df));
            if (!execBin.empty()) {
                candidates.push_back(execBin);
            }
        }
    }

    for (const char* binName : {
             "SkunkcraftsUpdater_lin",
             "SkunkcraftsUpdater-x86_64.AppImage",
             "SkunkcraftsUpdater.AppImage"}) {
        candidates.push_back(JoinPath(simRoot, binName));
        if (simRoot != ".") {
            candidates.push_back(JoinPath(".", binName));
        }
    }
#endif

    UpdaterAvailability fallbackLegacyApp;
    for (const auto& c : candidates) {
        if (!IsRegularFileOrBundle(c)) {
            continue;
        }
        bool isV4 = false;
        std::string ver = DetectUpdaterAppVersion(c, isV4);
        if (isV4) {
            out.isInstalled = true;
            out.isV4OrNewer = true;
            out.launchTarget = c;
            out.detectedVersion = ver;
            return out;
        }
        if (!fallbackLegacyApp.isInstalled) {
            fallbackLegacyApp.isInstalled = true;
            fallbackLegacyApp.isV4OrNewer = false;
            fallbackLegacyApp.hasDeprecatedPlugin = out.hasDeprecatedPlugin;
            fallbackLegacyApp.launchTarget = c;
            fallbackLegacyApp.detectedVersion = ver;
        }
    }

    if (fallbackLegacyApp.isInstalled) {
        return fallbackLegacyApp;
    }
    return out;
}

void LaunchSkunkCraftsUpdater(const UpdaterAvailability& info) {
    if (!info.isInstalled || info.launchTarget.empty()) {
        ImGui::OpenURL(kSkunkCraftsWebsiteURL);
        return;
    }

#if defined(_WIN32)
    std::filesystem::path exePath(info.launchTarget);
    std::string parentDir = exePath.parent_path().string();
    const char* workDir = !parentDir.empty() ? parentDir.c_str() : nullptr;
    ShellExecuteA(nullptr, "open", info.launchTarget.c_str(), nullptr, workDir, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = "open " + QuotePosixSingle(info.launchTarget);
    int rc = std::system(cmd.c_str());
    (void)rc;
#else
    std::filesystem::path binPath(info.launchTarget);
    std::string parentDir = binPath.parent_path().string();
    std::string cmd;
    if (!parentDir.empty()) {
        cmd = "cd " + QuotePosixSingle(parentDir) + " && " +
              QuotePosixSingle(info.launchTarget) + " >/dev/null 2>&1 &";
    } else {
        cmd = QuotePosixSingle(info.launchTarget) + " >/dev/null 2>&1 &";
    }
    int rc = std::system(cmd.c_str());
    (void)rc;
#endif
}

// ----------------------------------------------------------------------------
// Dotted Version Comparison (returns < 0 if lhs < rhs, 0 if equal, > 0 if lhs > rhs)
// ----------------------------------------------------------------------------

struct ParsedVersion {
    std::vector<int> numbers;
    int preReleaseRank = 100; // 100 = final release, 1 = alpha, 2 = beta, 3 = rc
    int preReleaseNum = 0;
};

ParsedVersion ParseVersionString(const std::string& raw) {
    ParsedVersion pv;
    std::string s = TrimWhitespace(raw);
    if (!s.empty() && (s[0] == 'v' || s[0] == 'V')) {
        s.erase(0, 1);
    }

    size_t idx = 0;
    while (idx < s.size()) {
        if (std::isdigit(static_cast<unsigned char>(s[idx]))) {
            int val = 0;
            while (idx < s.size() && std::isdigit(static_cast<unsigned char>(s[idx]))) {
                val = val * 10 + (s[idx] - '0');
                ++idx;
            }
            pv.numbers.push_back(val);
            if (idx < s.size() && (s[idx] == '.' || s[idx] == '_')) {
                ++idx;
                continue;
            }
        }
        break;
    }

    while (idx < s.size() && (s[idx] == '-' || s[idx] == '_' || s[idx] == ' ')) {
        ++idx;
    }

    if (idx < s.size() && std::isalpha(static_cast<unsigned char>(s[idx]))) {
        std::string tag;
        while (idx < s.size() && std::isalpha(static_cast<unsigned char>(s[idx]))) {
            tag.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(s[idx]))));
            ++idx;
        }
        if (tag == "a" || tag == "alpha") {
            pv.preReleaseRank = 1;
        } else if (tag == "b" || tag == "beta") {
            pv.preReleaseRank = 2;
        } else if (tag == "rc") {
            pv.preReleaseRank = 3;
        } else {
            pv.preReleaseRank = 50;
        }
        while (idx < s.size() && !std::isdigit(static_cast<unsigned char>(s[idx]))) {
            ++idx;
        }
        int sub = 0;
        while (idx < s.size() && std::isdigit(static_cast<unsigned char>(s[idx]))) {
            sub = sub * 10 + (s[idx] - '0');
            ++idx;
        }
        pv.preReleaseNum = sub;
    }

    return pv;
}

int CompareVersions(const std::string& lhs, const std::string& rhs) {
    if (TrimWhitespace(lhs) == TrimWhitespace(rhs)) {
        return 0;
    }
    ParsedVersion a = ParseVersionString(lhs);
    ParsedVersion b = ParseVersionString(rhs);
    size_t maxLen = std::max(a.numbers.size(), b.numbers.size());
    for (size_t i = 0; i < maxLen; ++i) {
        int va = (i < a.numbers.size()) ? a.numbers[i] : 0;
        int vb = (i < b.numbers.size()) ? b.numbers[i] : 0;
        if (va != vb) {
            return (va < vb) ? -1 : 1;
        }
    }
    if (a.preReleaseRank != b.preReleaseRank) {
        return (a.preReleaseRank < b.preReleaseRank) ? -1 : 1;
    }
    if (a.preReleaseNum != b.preReleaseNum) {
        return (a.preReleaseNum < b.preReleaseNum) ? -1 : 1;
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Self-Bootstrap & Active Channel Resolution (Zombie-Free)
// ----------------------------------------------------------------------------

void ShoreUpFlyWithLuaConfig(const std::string& pluginDir, const UpdaterAvailability& updater) {
    if (updater.isInstalled) {
        logMsg(logToDevCon, std::string("FlyWithLua Info: Detected SkunkCrafts Updater v") +
                            updater.detectedVersion + " (" + updater.launchTarget + ").");
        if (!updater.isV4OrNewer) {
            logMsg(logToDevCon,
                   "FlyWithLua Info: Older SkunkCrafts Updater v3.x detected -- upgrade to v4 for free at https://updater.skunkcrafts.com");
        }
    } else if (updater.hasDeprecatedPlugin) {
        logMsg(logToDevCon,
               "FlyWithLua Info: Legacy in-sim SkunkCraftsUpdater plugin detected -- install the modern SkunkCrafts Updater v4 app from https://updater.skunkcrafts.com");
    } else {
        logMsg(logToDevCon,
               "FlyWithLua Info: SkunkCrafts Updater not detected -- see https://updater.skunkcrafts.com for 1-click updates.");
    }

    const std::string mainCfg = JoinPath(pluginDir, "skunkcrafts_updater.cfg");
    const std::string skDir = JoinPath(pluginDir, "skunkcrafts");
    const std::string chDir = JoinPath(skDir, "channels");
    const std::string channelCfg = JoinPath(skDir, "channel.cfg");
    const std::string betaCfg = JoinPath(chDir, "beta.cfg");
    const std::string betaRemoved = JoinPath(chDir, "beta.cfg.removed");
    const std::string betaWithdrawn = JoinPath(chDir, "beta.cfg.withdrawn");

    // 1. Create skunkcrafts_updater.cfg if missing (e.g. partial manual zip extraction),
    //    or ensure public channel keys exist if an older file is present.
    if (!PathExists(mainCfg)) {
        std::ofstream ofs(mainCfg, std::ios::out | std::ios::trunc | std::ios::binary);
        if (ofs) {
            ofs << "zone|custom\n"
                << "disabled|false\n"
                << "module|" << kDepotReleaseURL << "\n"
                << "name|FlyWithLua\n"
                << "version|" << GetPluginVersionNo() << "\n"
                << "locked|false\n"
                << "main_name|release\n"
                << "channel.beta|" << kDepotBetaURL << "\n";
            ofs.close();
            logMsg(logToDevCon,
                   "FlyWithLua Info: skunkcrafts_updater.cfg was missing and has been created automatically.");
        } else {
            logMsg(logToDevCon,
                   "FlyWithLua Error: skunkcrafts_updater.cfg was missing and could not be created.");
            return;
        }
    } else {
        bool missingMainName = CfgValueFromFile(mainCfg, "main_name").empty();
        bool missingBetaKey = CfgValueFromFile(mainCfg, "channel.beta").empty();
        if (missingMainName || missingBetaKey) {
            EnsureEndsWithNewline(mainCfg);
            std::ofstream ofs(mainCfg, std::ios::out | std::ios::app | std::ios::binary);
            if (ofs) {
                if (missingMainName) {
                    ofs << "main_name|release\n";
                }
                if (missingBetaKey) {
                    ofs << "channel.beta|" << kDepotBetaURL << "\n";
                }
            }
        }
    }

    // 2. Initialize SCU 4.x skunkcrafts/channel.cfg ONLY when channel.cfg does not exist.
    //    Once SCU 4.x runs, switching back to Main writes "channel|stable" into channel.cfg
    //    (never deleting channel.cfg), so user channel selections are never overridden.
    std::error_code ec;
    if (!PathExists(channelCfg)) {
        std::filesystem::create_directories(chDir, ec);
        const char* initialChannel = "beta";
        const std::string legacyBetaCfg = JoinPath(pluginDir, "skunkcrafts_updater_beta.cfg");
        if (PathExists(legacyBetaCfg) && CfgValueFromFile(legacyBetaCfg, "enabled") == "false") {
            initialChannel = "stable";
        }
        std::ofstream ofs(channelCfg, std::ios::out | std::ios::trunc | std::ios::binary);
        if (ofs) {
            ofs << "channel|" << initialChannel << "\n";
            ofs.close();
            logMsg(logToDevCon,
                   std::string("FlyWithLua Info: Initialized SkunkCrafts 4.x channel configuration (") +
                   initialChannel + ").");
        }
    }

    // 3. Ensure skunkcrafts/channels/beta.cfg exists when channel.cfg is present,
    //    unless SCU explicitly wrote a .removed or .withdrawn tombstone.
    if (PathExists(channelCfg) && !PathExists(betaCfg) &&
        !PathExists(betaRemoved) && !PathExists(betaWithdrawn)) {
        std::filesystem::create_directories(chDir, ec);
        std::ofstream ofs(betaCfg, std::ios::out | std::ios::trunc | std::ios::binary);
        if (ofs) {
            ofs << "channel|beta\n"
                << "module|" << kDepotBetaURL << "\n"
                << "zone|custom\n"
                << "public|true\n";
        }
    }
}

std::string ResolveFollowedChannelURL(const std::string& pluginDir, bool& outIsBeta) {
    const std::string mainCfg = JoinPath(pluginDir, "skunkcrafts_updater.cfg");
    std::string mainURL = CfgValueFromFile(mainCfg, "module");
    if (mainURL.empty()) {
        mainURL = kDepotReleaseURL;
    }
    outIsBeta = false;

    const std::string channelCfg = JoinPath(pluginDir, "skunkcrafts/channel.cfg");
    if (PathExists(channelCfg)) {
        std::string chName = CfgValueFromFile(channelCfg, "channel");
        if (chName.empty() || chName == "stable") {
            return mainURL;
        }
        const std::string chFile = JoinPath(pluginDir, "skunkcrafts/channels/" + chName + ".cfg");
        if (PathExists(chFile)) {
            std::string chURL = CfgValueFromFile(chFile, "module");
            if (!chURL.empty()) {
                outIsBeta = true;
                return chURL;
            }
        }
        std::string inlineURL = CfgValueFromFile(mainCfg, ("channel." + chName).c_str());
        if (!inlineURL.empty()) {
            outIsBeta = true;
            return inlineURL;
        }
        return mainURL;
    }

    // Read-only fallback for legacy SCU 3.x skunkcrafts_updater_beta.cfg
    const std::string legacyBeta = JoinPath(pluginDir, "skunkcrafts_updater_beta.cfg");
    if (PathExists(legacyBeta) && CfgValueFromFile(legacyBeta, "enabled") != "false") {
        std::string betaURL = CfgValueFromFile(legacyBeta, "module");
        if (!betaURL.empty()) {
            outIsBeta = true;
            return betaURL;
        }
    }

    return mainURL;
}

// ----------------------------------------------------------------------------
// Cross-Platform HTTPS Text Fetch (5s Timeout, No Static libcurl Required)
// ----------------------------------------------------------------------------

#if !defined(_WIN32)
using ScuCURL = void;
using ScuCURLcode = int;
using ScuCURLoption = int;
using ScuCURLINFO = int;

constexpr int kScuCurlOk = 0;
constexpr int kScuCurlOptWriteData = 10001;
constexpr int kScuCurlOptUrl = 10002;
constexpr int kScuCurlOptTimeout = 13;
constexpr int kScuCurlOptUserAgent = 10018;
constexpr int kScuCurlOptWriteFunction = 20011;
constexpr int kScuCurlOptFollowLocation = 52;
constexpr int kScuCurlOptConnectTimeout = 78;
constexpr int kScuCurlOptNoSignal = 99;
constexpr int kScuCurlInfoResponseCode = 0x200002;

using ScuCurlInitFn = ScuCURL* (*)();
using ScuCurlSetoptFn = ScuCURLcode (*)(ScuCURL*, ScuCURLoption, ...);
using ScuCurlPerformFn = ScuCURLcode (*)(ScuCURL*);
using ScuCurlCleanupFn = void (*)(ScuCURL*);
using ScuCurlGetinfoFn = ScuCURLcode (*)(ScuCURL*, ScuCURLINFO, ...);

size_t ScuCurlWriteStringCB(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    auto* outStr = reinterpret_cast<std::string*>(userp);
    if (outStr->size() + total > 65536) {
        return 0; // Cap config reads at 64 KB
    }
    outStr->append(reinterpret_cast<const char*>(contents), total);
    return total;
}

bool FetchRemoteTextCurl(const std::string& url, const std::string& userAgent, std::string& outText) {
#if defined(__APPLE__)
    const char* dylibNames[] = { "libcurl.4.dylib", "/usr/lib/libcurl.4.dylib", "libcurl.dylib", nullptr };
#else
    const char* dylibNames[] = { "libcurl.so.4", "libcurl.so", nullptr };
#endif

    void* handle = nullptr;
    for (int i = 0; dylibNames[i] != nullptr; ++i) {
        handle = dlopen(dylibNames[i], RTLD_LAZY);
        if (handle) {
            break;
        }
    }
    if (!handle) {
        return false;
    }

    auto pInit = reinterpret_cast<ScuCurlInitFn>(dlsym(handle, "curl_easy_init"));
    auto pSetopt = reinterpret_cast<ScuCurlSetoptFn>(dlsym(handle, "curl_easy_setopt"));
    auto pPerform = reinterpret_cast<ScuCurlPerformFn>(dlsym(handle, "curl_easy_perform"));
    auto pCleanup = reinterpret_cast<ScuCurlCleanupFn>(dlsym(handle, "curl_easy_cleanup"));
    auto pGetinfo = reinterpret_cast<ScuCurlGetinfoFn>(dlsym(handle, "curl_easy_getinfo"));

    if (!pInit || !pSetopt || !pPerform || !pCleanup) {
        dlclose(handle);
        return false;
    }

    ScuCURL* curl = pInit();
    if (!curl) {
        dlclose(handle);
        return false;
    }

    outText.clear();
    pSetopt(curl, kScuCurlOptNoSignal, 1L);
    pSetopt(curl, kScuCurlOptConnectTimeout, 4L);
    pSetopt(curl, kScuCurlOptTimeout, 5L);
    pSetopt(curl, kScuCurlOptFollowLocation, 1L);
    pSetopt(curl, kScuCurlOptUserAgent, userAgent.c_str());
    pSetopt(curl, kScuCurlOptUrl, url.c_str());
    pSetopt(curl, kScuCurlOptWriteFunction, ScuCurlWriteStringCB);
    pSetopt(curl, kScuCurlOptWriteData, &outText);

    ScuCURLcode res = pPerform(curl);
    long httpCode = 0;
    if (res == kScuCurlOk && pGetinfo) {
        pGetinfo(curl, kScuCurlInfoResponseCode, &httpCode);
    }

    pCleanup(curl);
    dlclose(handle);

    return (res == kScuCurlOk && httpCode == 200 && !outText.empty());
}

bool FetchRemoteTextCLI(const std::string& url, std::string& outText) {
    if (url.find_first_of("'\"`;\r\n") != std::string::npos) {
        return false;
    }
    std::string cmd = "curl -fsSL --connect-timeout 4 --max-time 5 '" + url + "'";
    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) {
        return false;
    }
    outText.clear();
    char chunk[2048];
    size_t n = 0;
    while ((n = std::fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (outText.size() + n > 65536) {
            pclose(fp);
            return false;
        }
        outText.append(chunk, n);
    }
    int status = pclose(fp);
    return (status == 0 && !outText.empty());
}
#endif // !defined(_WIN32)

#if defined(_WIN32)
bool FetchRemoteTextWinINet(const std::string& url, const std::string& userAgent, std::string& outText) {
    HMODULE hWinINet = LoadLibraryA("wininet.dll");
    if (!hWinINet) {
        return false;
    }

    using InternetOpenAFn = void* (__stdcall*)(const char*, unsigned long, const char*, const char*, unsigned long);
    using InternetSetOptionAFn = int (__stdcall*)(void*, unsigned long, void*, unsigned long);
    using InternetOpenUrlAFn = void* (__stdcall*)(void*, const char*, const char*, unsigned long, unsigned long, size_t);
    using HttpQueryInfoAFn = int (__stdcall*)(void*, unsigned long, void*, unsigned long*, unsigned long*);
    using InternetReadFileFn = int (__stdcall*)(void*, void*, unsigned long, unsigned long*);
    using InternetCloseHandleFn = int (__stdcall*)(void*);

    auto pOpen = reinterpret_cast<InternetOpenAFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "InternetOpenA")));
    auto pSetOpt = reinterpret_cast<InternetSetOptionAFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "InternetSetOptionA")));
    auto pOpenUrl = reinterpret_cast<InternetOpenUrlAFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "InternetOpenUrlA")));
    auto pQuery = reinterpret_cast<HttpQueryInfoAFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "HttpQueryInfoA")));
    auto pRead = reinterpret_cast<InternetReadFileFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "InternetReadFile")));
    auto pClose = reinterpret_cast<InternetCloseHandleFn>(reinterpret_cast<void*>(GetProcAddress(hWinINet, "InternetCloseHandle")));

    if (!pOpen || !pOpenUrl || !pRead || !pClose) {
        FreeLibrary(hWinINet);
        return false;
    }

    void* hInternet = pOpen(userAgent.c_str(), 0, nullptr, nullptr, 0);
    if (!hInternet) {
        FreeLibrary(hWinINet);
        return false;
    }

    if (pSetOpt) {
        unsigned long connectMs = 4000;
        unsigned long recvMs = 5000;
        pSetOpt(hInternet, 2 /* INTERNET_OPTION_CONNECT_TIMEOUT */, &connectMs, sizeof(connectMs));
        pSetOpt(hInternet, 6 /* INTERNET_OPTION_RECEIVE_TIMEOUT */, &recvMs, sizeof(recvMs));
    }

    // INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | optional INTERNET_FLAG_SECURE
    unsigned long flags = 0x80000000UL | 0x04000000UL;
    if (url.rfind("https://", 0) == 0) {
        flags |= 0x00800000UL;
    }

    void* hUrl = pOpenUrl(hInternet, url.c_str(), nullptr, 0, flags, 0);
    if (!hUrl) {
        pClose(hInternet);
        FreeLibrary(hWinINet);
        return false;
    }

    if (pQuery) {
        unsigned long statusCode = 0;
        unsigned long statusLen = sizeof(statusCode);
        unsigned long index = 0;
        // HTTP_QUERY_STATUS_CODE (19) | HTTP_QUERY_FLAG_NUMBER (0x20000000)
        if (pQuery(hUrl, 19UL | 0x20000000UL, &statusCode, &statusLen, &index) && statusCode != 200) {
            pClose(hUrl);
            pClose(hInternet);
            FreeLibrary(hWinINet);
            return false;
        }
    }

    outText.clear();
    char chunk[2048];
    unsigned long bytesRead = 0;
    while (pRead(hUrl, chunk, sizeof(chunk), &bytesRead) && bytesRead > 0) {
        if (outText.size() + bytesRead > 65536) {
            pClose(hUrl);
            pClose(hInternet);
            FreeLibrary(hWinINet);
            return false;
        }
        outText.append(chunk, bytesRead);
    }

    pClose(hUrl);
    pClose(hInternet);
    FreeLibrary(hWinINet);
    return !outText.empty();
}
#endif // defined(_WIN32)

bool FetchRemoteText(const std::string& url, const std::string& userAgent, std::string& outText) {
#if defined(_WIN32)
    return FetchRemoteTextWinINet(url, userAgent, outText);
#else
    if (FetchRemoteTextCurl(url, userAgent, outText)) {
        return true;
    }
    return FetchRemoteTextCLI(url, outText);
#endif
}

// ----------------------------------------------------------------------------
// Async Check State & Preferences Persistence
// ----------------------------------------------------------------------------

enum class CheckPhase {
    Idle,
    Checking,
    Succeeded,
    Failed
};

struct UpdateState {
    std::mutex mtx;
    std::thread worker;
    CheckPhase phase = CheckPhase::Idle;
    bool isBetaChannel = true;
    bool updateAvailable = false;
    UpdaterAvailability updaterInfo;
    bool autoAlertEvaluated = false;
    bool sessionDismissed = false;
    int cockpitReadyTicks = 0;
    std::string localVersion;
    std::string remoteVersion;
    std::string checkedURL;
    std::string errorSummary;
    std::vector<std::string> pendingLogs;
};

UpdateState gState;

bool ShouldShowAutoAlertForVersion(const std::string& remoteVer) {
    if (check_for_updates == 0) {
        return false;
    }
    if (!skipped_update_version.empty() &&
        CompareVersions(skipped_update_version, remoteVer) == 0) {
        return false;
    }
    std::time_t now = std::time(nullptr);
    if (remind_update_after_epoch > 0 && static_cast<long long>(now) < remind_update_after_epoch) {
        return false;
    }
    return true;
}

void StartAsyncCheckInternal(bool forceManualCheck) {
    std::string pluginDir = !pluginMainDir.empty()
                                ? pluginMainDir
                                : std::string("./Resources/plugins/FlyWithLua");
    bool isBeta = false;
    std::string channelURL = ResolveFollowedChannelURL(pluginDir, isBeta);
    while (!channelURL.empty() && channelURL.back() == '/') {
        channelURL.pop_back();
    }
    std::string fetchURL = channelURL + "/skunkcrafts_updater.cfg";
    const std::string mainCfg = JoinPath(pluginDir, "skunkcrafts_updater.cfg");
    std::string cfgVer = CfgValueFromFile(mainCfg, "version");
    std::string localVer = !cfgVer.empty() ? cfgVer : std::string(GetPluginVersionNo());
    std::string userAgent = std::string("FlyWithLua/") + GetPluginVersionNo();

    std::thread oldWorker;
    {
        std::lock_guard<std::mutex> lock(gState.mtx);
        if (gState.phase == CheckPhase::Checking) {
            return;
        }
        if (gState.worker.joinable()) {
            oldWorker = std::move(gState.worker);
        }
        gState.phase = CheckPhase::Checking;
        gState.isBetaChannel = isBeta;
        gState.localVersion = localVer;
        gState.checkedURL = fetchURL;
        gState.errorSummary.clear();
        if (forceManualCheck) {
            gState.autoAlertEvaluated = true; // Manual check opens window directly
        } else {
            gState.autoAlertEvaluated = false;
        }
    }
    if (oldWorker.joinable()) {
        oldWorker.join();
    }

    gState.worker = std::thread([fetchURL, userAgent, localVer, isBeta]() {
        std::string body;
        bool ok = FetchRemoteText(fetchURL, userAgent, body);
        std::string remoteName = ok ? CfgValueFromText(body, "name") : "";
        std::string remoteVer = ok ? CfgValueFromText(body, "version") : "";

        std::lock_guard<std::mutex> lock(gState.mtx);
        const char* chLabel = isBeta ? "beta" : "release";
        if (!ok || remoteVer.empty() || (!remoteName.empty() && remoteName != "FlyWithLua")) {
            gState.phase = CheckPhase::Failed;
            gState.updateAvailable = false;
            gState.errorSummary = "Unable to reach SkunkCrafts " + std::string(chLabel) + " depot.";
            gState.pendingLogs.push_back(
                "FlyWithLua Info: Could not fetch SkunkCrafts version info from " +
                std::string(chLabel) + " depot (" + fetchURL + ").");
            return;
        }

        gState.remoteVersion = remoteVer;
        gState.updateAvailable = (CompareVersions(localVer, remoteVer) < 0);
        gState.phase = CheckPhase::Succeeded;

        if (gState.updateAvailable) {
            gState.pendingLogs.push_back(
                "FlyWithLua Info: New version v" + remoteVer + " is available on SkunkCrafts " +
                std::string(chLabel) + " channel (installed: v" + localVer +
                "). Run SkunkCrafts Updater to update.");
        } else {
            gState.pendingLogs.push_back(
                "FlyWithLua Info: Up to date on SkunkCrafts " + std::string(chLabel) +
                " channel (installed: v" + localVer + ", depot: v" + remoteVer + ").");
        }
    });
}

// ----------------------------------------------------------------------------
// Native C++ ImGui Update Alert & Status Card
// ----------------------------------------------------------------------------

class FWLUpdateAlertWindow : public ImgWindow {
public:
    FWLUpdateAlertWindow()
        : ImgWindow(
              MakeCenteredTopGeometry(540, 245),
              xplm_WindowDecorationRoundRectangle,
              xplm_WindowLayerFloatingWindows,
              flywithlua::imgui_mouse_cursors != 0) {
        SetWindowTitle("FlyWithLua -- Update Check");
        SetWindowResizingLimits(480, 215, 760, 400);
        SetVisible(true);
    }

    ~FWLUpdateAlertWindow() override = default;

    void SyncVR() {
        moveForVR();
    }

protected:
    ImGuiWindowFlags_ beforeBegin() override {
        return static_cast<ImGuiWindowFlags_>(ImGuiWindowFlags_NoCollapse);
    }

    void buildInterface() override {
        CheckPhase phase = CheckPhase::Idle;
        bool isBeta = true;
        bool hasUpdate = false;
        UpdaterAvailability updater;
        std::string localVer;
        std::string remoteVer;
        std::string errText;
        {
            std::lock_guard<std::mutex> lock(gState.mtx);
            phase = gState.phase;
            isBeta = gState.isBetaChannel;
            hasUpdate = gState.updateAvailable;
            updater = gState.updaterInfo;
            localVer = gState.localVersion;
            remoteVer = gState.remoteVersion;
            errText = gState.errorSummary;
        }

        ImFont* headingFont = flywithlua::GetFontByName("Roboto-Bold-20");
        if (!headingFont) {
            headingFont = flywithlua::GetFontByName("Roboto-Medium-20");
        }
        ImFont* bodyFont = flywithlua::GetFontByName("Roboto-Regular-16");
        if (!bodyFont) {
            bodyFont = flywithlua::GetDefaultFont();
        }
        if (bodyFont) {
            ImGui::PushFont(bodyFont);
        }

        ImFont* boldBodyFont = flywithlua::GetFontByName("Roboto-Bold-16");
        if (!boldBodyFont) {
            boldBodyFont = flywithlua::GetFontByName("Roboto-Medium-16");
        }
        const char* chName = isBeta ? "beta" : "release";
        const char* chTitle = isBeta ? "Beta" : "Release";
        const ImVec4 chColor = isBeta
                                   ? ImVec4(0.96f, 0.78f, 0.35f, 1.0f)
                                   : ImVec4(0.62f, 0.84f, 1.0f, 1.0f);

        if (phase == CheckPhase::Checking) {
            if (headingFont) {
                ImGui::PushFont(headingFont);
            }
            ImGui::TextColored(ImVec4(0.55f, 0.82f, 1.0f, 1.0f), "Checking for FlyWithLua Updates...");
            if (headingFont) {
                ImGui::PopFont();
            }
            ImGui::Spacing();
            ImGui::TextWrapped("Contacting the FlyWithLua SkunkCrafts %s channel depot...", chName);
        } else if (phase == CheckPhase::Failed) {
            if (headingFont) {
                ImGui::PushFont(headingFont);
            }
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.35f, 1.0f), "Update Check Unavailable");
            if (headingFont) {
                ImGui::PopFont();
            }
            ImGui::Spacing();
            ImGui::TextWrapped("%s", errText.c_str());
            ImGui::Spacing();
            if (ImGui::Button("Retry Now")) {
                UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
                {
                    std::lock_guard<std::mutex> lock(gState.mtx);
                    gState.updaterInfo = refreshed;
                }
                StartAsyncCheckInternal(true);
            }
            ApplyHandCursorAndTooltip("Retry checking the remote SkunkCrafts depot now.");
            ImGui::SameLine();
            if (ImGui::ButtonLink("Open X-Plane.org Download Page")) {
                ImGui::OpenURL(kOrgDownloadURL);
            }
            ImGui::TimedTooltip::TextUnformatted("Open the official FlyWithLua NG+ page in your browser.");
        } else if (hasUpdate) {
            if (headingFont) {
                ImGui::PushFont(headingFont);
            }
            ImGui::TextColored(ImVec4(0.35f, 0.92f, 0.55f, 1.0f),
                               "FlyWithLua v%s is Available!", remoteVer.c_str());
            if (headingFont) {
                ImGui::PopFont();
            }
            ImGui::TextDisabled("Installed: v%s   |   Channel:", localVer.c_str());
            ImGui::SameLine(0.0f, 5.0f);
            if (boldBodyFont) {
                ImGui::PushFont(boldBodyFont);
            }
            ImGui::TextColored(chColor, "%s", chTitle);
            if (boldBodyFont) {
                ImGui::PopFont();
            }
            ImGui::Separator();
            ImGui::Spacing();

            if (updater.isInstalled) {
                ImGui::TextWrapped(
                    "A newer version of FlyWithLua is ready on the %s channel. "
                    "Close X-Plane before running Update in SkunkCrafts Updater so plugin binaries can be replaced cleanly.",
                    chName);
                ImGui::Spacing();

                if (ImGui::Button("Launch SkunkCrafts Updater")) {
                    UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
                    {
                        std::lock_guard<std::mutex> lock(gState.mtx);
                        gState.updaterInfo = refreshed;
                    }
                    LaunchSkunkCraftsUpdater(refreshed);
                }
                ApplyHandCursorAndTooltip(
                    "Open SkunkCrafts Updater (remember to quit X-Plane before updating).");
                ImGui::SameLine();

                if (ImGui::ButtonLink("X-Plane.org Download Page")) {
                    ImGui::OpenURL(kOrgDownloadURL);
                }
                ImGui::TimedTooltip::TextUnformatted("Prefer manual installs? Open the FlyWithLua NG+ download page on X-Plane.org.");

                if (!updater.isV4OrNewer) {
                    ImGui::Spacing();
                    ImGui::TextColored(
                        ImVec4(0.96f, 0.78f, 0.35f, 1.0f),
                        "Tip: You have SkunkCrafts Updater v%s -- v4 is available free with multi-sim & public beta support:",
                        updater.detectedVersion.c_str());
                    if (ImGui::ButtonLink("Upgrade to SkunkCrafts Updater v4")) {
                        ImGui::OpenURL(kSkunkCraftsWebsiteURL);
                    }
                    ImGui::TimedTooltip::TextUnformatted("Open updater.skunkcrafts.com to download SkunkCrafts Updater v4.");
                }
            } else {
                if (updater.hasDeprecatedPlugin) {
                    ImGui::TextColored(
                        ImVec4(0.96f, 0.78f, 0.35f, 1.0f),
                        "Note: The old in-sim SkunkCraftsUpdater plugin is retired and cannot replace running plugin binaries.");
                    ImGui::Spacing();
                }
                ImGui::TextWrapped(
                    "FlyWithLua supports fast 1-click updates with the free SkunkCrafts Updater v4 app, "
                    "or you can download the new .zip manually from X-Plane.org.");
                ImGui::Spacing();

                if (ImGui::ButtonLink("Get SkunkCrafts Updater v4 (Free)")) {
                    ImGui::OpenURL(kSkunkCraftsWebsiteURL);
                }
                ImGui::TimedTooltip::TextUnformatted(
                    "Open updater.skunkcrafts.com to download the free SkunkCrafts Updater v4 app.");

                ImGui::SameLine();
                if (ImGui::ButtonLink("Download .zip on X-Plane.org")) {
                    ImGui::OpenURL(kOrgDownloadURL);
                }
                ImGui::TimedTooltip::TextUnformatted("Open the official FlyWithLua NG+ download page on X-Plane.org.");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Not Now")) {
                DismissForSession();
            }
            ApplyHandCursorAndTooltip("Close this notice for the current X-Plane session.");

            ImGui::SameLine();
            if (ImGui::Button("Remind in 7 Days")) {
                long long untilEpoch = static_cast<long long>(std::time(nullptr)) + (7LL * 86400LL);
                remind_update_after_epoch = untilEpoch;
                UpdatePrefInIniFile("RemindUpdateAfterEpoch", std::to_string(untilEpoch));
                DismissForSession();
            }
            ApplyHandCursorAndTooltip("Snooze update alerts for 7 days (saved in fwl_prefs.ini).");

            ImGui::SameLine();
            std::string skipLabel = "Skip v" + remoteVer;
            if (ImGui::Button(skipLabel.c_str())) {
                skipped_update_version = remoteVer;
                UpdatePrefInIniFile("SkippedUpdateVersion", remoteVer);
                DismissForSession();
            }
            ApplyHandCursorAndTooltip(
                "Do not notify again for this version; alerts will resume when a newer release comes out.");
        } else {
            if (headingFont) {
                ImGui::PushFont(headingFont);
            }
            ImGui::TextColored(ImVec4(0.45f, 0.88f, 0.62f, 1.0f),
                               "FlyWithLua is Up to Date (v%s)", localVer.c_str());
            if (headingFont) {
                ImGui::PopFont();
            }
            ImGui::TextDisabled("Active SkunkCrafts channel:");
            ImGui::SameLine(0.0f, 5.0f);
            if (boldBodyFont) {
                ImGui::PushFont(boldBodyFont);
            }
            ImGui::TextColored(chColor, "%s", chTitle);
            if (boldBodyFont) {
                ImGui::PopFont();
            }
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextDisabled("   |   Depot version: v%s",
                                remoteVer.empty() ? localVer.c_str() : remoteVer.c_str());
            ImGui::Separator();
            ImGui::Spacing();

            if (updater.isInstalled) {
                ImGui::TextWrapped("SkunkCrafts Updater (v%s) is installed and ready to manage FlyWithLua updates.",
                                   updater.detectedVersion.c_str());
                if (!updater.isV4OrNewer) {
                    ImGui::Spacing();
                    ImGui::TextColored(
                        ImVec4(0.96f, 0.78f, 0.35f, 1.0f),
                        "Tip: SkunkCrafts Updater v4 is available free at updater.skunkcrafts.com.");
                }
                ImGui::Spacing();
                if (ImGui::Button("Launch SkunkCrafts Updater")) {
                    UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
                    {
                        std::lock_guard<std::mutex> lock(gState.mtx);
                        gState.updaterInfo = refreshed;
                    }
                    LaunchSkunkCraftsUpdater(refreshed);
                }
                ApplyHandCursorAndTooltip("Open SkunkCrafts Updater.");
                if (!updater.isV4OrNewer) {
                    ImGui::SameLine();
                    if (ImGui::ButtonLink("Get v4 Upgrade")) {
                        ImGui::OpenURL(kSkunkCraftsWebsiteURL);
                    }
                    ImGui::TimedTooltip::TextUnformatted("Open updater.skunkcrafts.com to download SkunkCrafts Updater v4.");
                }
            } else {
                if (updater.hasDeprecatedPlugin) {
                    ImGui::TextColored(
                        ImVec4(0.96f, 0.78f, 0.35f, 1.0f),
                        "Note: The old in-sim SkunkCraftsUpdater plugin is retired -- install the v4 app instead.");
                    ImGui::Spacing();
                }
                ImGui::TextWrapped(
                    "Tip: Install the free SkunkCrafts Updater v4 app "
                    "for 1-click FlyWithLua updates and repairs.");
                ImGui::Spacing();
                if (ImGui::ButtonLink("Get SkunkCrafts Updater v4 (Free)")) {
                    ImGui::OpenURL(kSkunkCraftsWebsiteURL);
                }
                ImGui::TimedTooltip::TextUnformatted("Open updater.skunkcrafts.com in your browser.");
            }

            ImGui::SameLine();
            if (ImGui::Button("Re-check Now")) {
                UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
                {
                    std::lock_guard<std::mutex> lock(gState.mtx);
                    gState.updaterInfo = refreshed;
                }
                StartAsyncCheckInternal(true);
            }
            ApplyHandCursorAndTooltip("Query the remote SkunkCrafts channel depot again.");

            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                SetVisible(false);
            }
            ApplyHandCursorAndTooltip("Close this window.");
        }

        ImGui::Spacing();
        bool autoCheck = (check_for_updates != 0);
        if (ImGui::Checkbox("Check for FlyWithLua updates automatically at startup", &autoCheck)) {
            check_for_updates = autoCheck ? 1 : 0;
            UpdatePrefInIniFile("CheckForUpdates", autoCheck ? "1" : "0");
        }
        ApplyHandCursorAndTooltip("Saved as CheckForUpdates in Resources/plugins/FlyWithLua/fwl_prefs.ini.");

        if (bodyFont) {
            ImGui::PopFont();
        }
    }

private:
    static WindowGeometry MakeCenteredTopGeometry(int width, int height) {
        int sl = 0, st = 0, sr = 1920, sb = 0;
        XPLMGetScreenBoundsGlobal(&sl, &st, &sr, &sb);
        int screenW = std::max(640, sr - sl);
        int left = sl + (screenW - width) / 2;
        int topClearance = std::max(90, ImgWindow::GetTopMargin(xplm_WindowDecorationRoundRectangle) + 35);
        int top = st - topClearance;
        return WindowGeometry::FromTopLeft(left, top, width, height);
    }

    static void ApplyHandCursorAndTooltip(const char* tip) {
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if (tip && tip[0] != '\0') {
            ImGui::TimedTooltip::TextUnformatted(tip);
        }
    }

    void DismissForSession() {
        {
            std::lock_guard<std::mutex> lock(gState.mtx);
            gState.sessionDismissed = true;
        }
        SetVisible(false);
    }
};

std::shared_ptr<FWLUpdateAlertWindow> gAlertWindow;

void EnsureAlertWindowOpen() {
    if (!ImgWindow::sFontAtlas) {
        return;
    }
    if (!gAlertWindow) {
        gAlertWindow = std::make_shared<FWLUpdateAlertWindow>();
    } else {
        gAlertWindow->SetVisible(true);
        if (!gAlertWindow->IsInVR()) {
            gAlertWindow->BringWindowToFront();
        }
    }
}

} // namespace

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

bool UpdatePrefInIniFile(const std::string& key, const std::string& value) {
    std::vector<std::string> lines;
    bool updatedExisting = false;

    {
        std::ifstream ifs(kPrefsFilePath, std::ios::in);
        if (ifs) {
            std::string line;
            while (std::getline(ifs, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                std::string trimmed = TrimWhitespace(line);
                if (!trimmed.empty() && trimmed[0] != ';' && trimmed[0] != '#') {
                    size_t eqPos = trimmed.find('=');
                    if (eqPos != std::string::npos) {
                        std::string existingKey = TrimWhitespace(trimmed.substr(0, eqPos));
                        if (existingKey == key) {
                            line = key + " = " + value;
                            updatedExisting = true;
                        }
                    }
                }
                lines.push_back(line);
            }
        }
    }

    if (!updatedExisting) {
        if (!lines.empty() && !lines.back().empty()) {
            lines.push_back("");
        }
        lines.push_back(key + " = " + value);
    }

    std::ofstream ofs(kPrefsFilePath, std::ios::out | std::ios::trunc);
    if (!ofs) {
        return false;
    }
    for (const auto& l : lines) {
        ofs << l << "\n";
    }
    return true;
}

void OnPluginEnable() {
    std::string pluginDir = !pluginMainDir.empty()
                                ? pluginMainDir
                                : std::string("./Resources/plugins/FlyWithLua");
    UpdaterAvailability updater = DetectSkunkCraftsUpdater();
    {
        std::lock_guard<std::mutex> lock(gState.mtx);
        gState.updaterInfo = updater;
        gState.cockpitReadyTicks = 0;
        gState.sessionDismissed = false;
    }

    ShoreUpFlyWithLuaConfig(pluginDir, updater);

    if (check_for_updates != 0) {
        StartAsyncCheckInternal(false);
    }
}

void OnFlightLoopTick() {
    std::vector<std::string> logsToFlush;
    bool shouldPopAutoAlert = false;

    {
        std::lock_guard<std::mutex> lock(gState.mtx);
        if (!gState.pendingLogs.empty()) {
            logsToFlush.swap(gState.pendingLogs);
        }
        if (gState.cockpitReadyTicks < 10) {
            ++gState.cockpitReadyTicks;
        }
        if (!gState.autoAlertEvaluated &&
            gState.phase == CheckPhase::Succeeded &&
            gState.cockpitReadyTicks >= 3) {
            gState.autoAlertEvaluated = true;
            if (gState.updateAvailable &&
                !gState.sessionDismissed &&
                ShouldShowAutoAlertForVersion(gState.remoteVersion)) {
                shouldPopAutoAlert = true;
            }
        }
    }

    for (const auto& msg : logsToFlush) {
        logMsg(logToDevCon, msg);
    }

    if (shouldPopAutoAlert) {
        UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
        {
            std::lock_guard<std::mutex> lock(gState.mtx);
            gState.updaterInfo = refreshed;
        }
        EnsureAlertWindowOpen();
    }

    if (gAlertWindow) {
        if (!gAlertWindow->GetVisible()) {
            gAlertWindow.reset();
        } else {
            gAlertWindow->SyncVR();
        }
    }
}

void OpenUpdateStatusWindow() {
    UpdaterAvailability refreshed = DetectSkunkCraftsUpdater();
    {
        std::lock_guard<std::mutex> lock(gState.mtx);
        gState.updaterInfo = refreshed;
    }
    StartAsyncCheckInternal(true);
    EnsureAlertWindowOpen();
}

void CloseAlertWindow() {
    if (gAlertWindow) {
        gAlertWindow->SetVisible(false);
        gAlertWindow.reset();
    }
}

void StopAsyncWorker() {
    CloseAlertWindow();
    std::thread workerToJoin;
    {
        std::lock_guard<std::mutex> lock(gState.mtx);
        if (gState.worker.joinable()) {
            workerToJoin = std::move(gState.worker);
        }
    }
    if (workerToJoin.joinable()) {
        workerToJoin.join();
    }
}

} // namespace scu
} // namespace flywithlua
