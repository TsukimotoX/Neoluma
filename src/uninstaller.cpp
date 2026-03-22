// This uninstaller is only available on Windows.
// Unix systems like Mac or Linux contain uninstall.sh at ~/.local/bin/neoluma
// Run it, if you wish to uninstall Neoluma.

// Neoluma Uninstaller (Windows only)
// - Requires admin rights
// - Finds install dir (Program Files / LocalAppData / PATH)
// - Removes PATH entries (User + System)
// - Removes registry "Installed apps" entry (org.astrahelm.neoluma)
// - Deletes install directory
// - Uses a temp-copy cleanup worker so it can delete itself if it lives inside install dir

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// -----------------------------
// Config
// -----------------------------
static constexpr const wchar_t* kUninstallId = L"org.astrahelm.neoluma";

// -----------------------------
// Console helpers
// -----------------------------
static void waitForEnter()
{
    std::wcout << L"\nPress Enter to close...";
    std::wstring dummy;
    std::getline(std::wcin, dummy);
}

// -----------------------------
// String helpers
// -----------------------------
static std::wstring trim(std::wstring s)
{
    auto isWs = [](wchar_t c) { return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n'; };
    while (!s.empty() && isWs(s.front())) s.erase(s.begin());
    while (!s.empty() && isWs(s.back())) s.pop_back();
    return s;
}

static std::wstring toLower(std::wstring s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return s;
}

static std::wstring normalizePath(std::wstring p)
{
    p = trim(p);

    if (p.size() >= 2 && p.front() == L'"' && p.back() == L'"')
        p = p.substr(1, p.size() - 2);

    std::replace(p.begin(), p.end(), L'/', L'\\');

    while (p.size() > 3 && p.back() == L'\\') p.pop_back();
    return toLower(p);
}

static std::vector<std::wstring> splitPathList(const std::wstring& pathValue)
{
    std::vector<std::wstring> parts;
    std::wstring cur;
    for (wchar_t c : pathValue)
    {
        if (c == L';')
        {
            parts.push_back(cur);
            cur.clear();
        }
        else
        {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

static std::wstring joinPathList(const std::vector<std::wstring>& parts)
{
    std::wstring out;
    for (size_t i = 0; i < parts.size(); i++)
    {
        if (i) out += L";";
        out += parts[i];
    }
    return out;
}

// -----------------------------
// Win32 path/env helpers
// -----------------------------
static std::wstring getSelfPath()
{
    wchar_t buf[32768]{};
    DWORD n = GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)));
    if (n == 0 || n >= std::size(buf)) return L"";
    return std::wstring(buf, n);
}

static std::wstring getTempDir()
{
    wchar_t buf[32768]{};
    DWORD n = GetTempPathW(static_cast<DWORD>(std::size(buf)), buf);
    if (n == 0 || n >= std::size(buf)) return L"";
    return std::wstring(buf, n);
}

static std::wstring getEnvVar(const wchar_t* name)
{
    DWORD need = GetEnvironmentVariableW(name, nullptr, 0);
    if (need == 0) return L"";

    std::wstring out(need, L'\0');
    GetEnvironmentVariableW(name, out.data(), need);
    while (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

static void broadcastEnvChange()
{
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                        reinterpret_cast<LPARAM>(L"Environment"),
                        SMTO_ABORTIFHUNG, 2000, nullptr);
}

static void broadcastUninstallChange()
{
    // Helps Settings/Explorer refresh installed apps list faster (best-effort)
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                        reinterpret_cast<LPARAM>(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
                        SMTO_ABORTIFHUNG, 2000, nullptr);
}

// -----------------------------
// Admin elevation
// -----------------------------
static bool isRunningAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                0, 0, 0, 0, 0, 0, &adminGroup))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
}

static bool relaunchAsAdmin(int argc, wchar_t** argv)
{
    std::wstring exePath = getSelfPath();
    if (exePath.empty()) return false;

    std::wstring args;
    for (int i = 1; i < argc; i++)
    {
        if (!args.empty()) args += L" ";
        args += L"\"";
        args += argv[i];
        args += L"\"";
    }

    HINSTANCE res = ShellExecuteW(nullptr, L"runas", exePath.c_str(),
                                 args.empty() ? nullptr : args.c_str(),
                                 nullptr, SW_SHOWNORMAL);
    return (INT_PTR)res > 32;
}

// -----------------------------
// Registry helpers
// -----------------------------
static bool readRegString(HKEY root, const std::wstring& subKey,
                          const wchar_t* valueName, std::wstring& outValue, DWORD& outType)
{
    outValue.clear();
    outType = 0;

    DWORD type = 0;
    DWORD bytes = 0;

    LONG rc = RegGetValueW(root, subKey.c_str(), valueName,
                           RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ,
                           &type, nullptr, &bytes);
    if (rc != ERROR_SUCCESS || bytes == 0) return false;

    std::wstring buf(bytes / sizeof(wchar_t), L'\0');
    rc = RegGetValueW(root, subKey.c_str(), valueName,
                      RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ,
                      &type, buf.data(), &bytes);
    if (rc != ERROR_SUCCESS) return false;

    while (!buf.empty() && buf.back() == L'\0') buf.pop_back();

    outValue = buf;
    outType = type;
    return true;
}

static bool writeRegString(HKEY root, const wchar_t* subKey, const wchar_t* valueName,
                           const std::wstring& value, DWORD type)
{
    HKEY hKey{};
    if (RegOpenKeyExW(root, subKey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD bytes = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
    LONG rc = RegSetValueExW(hKey, valueName, 0, type,
                            reinterpret_cast<const BYTE*>(value.c_str()), bytes);
    RegCloseKey(hKey);
    return rc == ERROR_SUCCESS;
}

// IMPORTANT: delete child key from its PARENT (this actually removes the uninstall entry)
static bool deleteRegKeyRecursive(HKEY root, const std::wstring& parentSubKey, const std::wstring& childKeyName)
{
    HKEY parent{};
    if (RegOpenKeyExW(root, parentSubKey.c_str(), 0, KEY_READ | KEY_WRITE, &parent) != ERROR_SUCCESS)
        return false;

    // Delete all subkeys of child first (best-effort)
    HKEY child{};
    if (RegOpenKeyExW(parent, childKeyName.c_str(), 0, KEY_READ | KEY_WRITE, &child) == ERROR_SUCCESS)
    {
        RegDeleteTreeW(child, nullptr);
        RegCloseKey(child);
    }

    LONG rc = RegDeleteKeyW(parent, childKeyName.c_str());
    RegCloseKey(parent);

    return rc == ERROR_SUCCESS;
}

static bool removeEntryFromPath(HKEY root, const wchar_t* subKey, const std::wstring& entryToRemove)
{
    std::wstring pathValue;
    DWORD type = 0;

    if (!readRegString(root, subKey, L"Path", pathValue, type))
        return true; // nothing to do

    auto parts = splitPathList(pathValue);

    std::wstring target = normalizePath(entryToRemove);
    bool changed = false;

    std::vector<std::wstring> kept;
    kept.reserve(parts.size());

    for (auto& p : parts)
    {
        std::wstring t = trim(p);
        if (t.empty()) continue;

        if (normalizePath(t) == target)
        {
            changed = true;
            continue;
        }
        kept.push_back(t);
    }

    if (!changed) return true;

    std::wstring newValue = joinPathList(kept);
    return writeRegString(root, subKey, L"Path", newValue, type);
}

static std::vector<std::wstring> getPathEntriesFromRegistry(HKEY root, const wchar_t* subKey)
{
    std::wstring pathValue;
    DWORD type = 0;
    if (!readRegString(root, subKey, L"Path", pathValue, type)) return {};
    return splitPathList(pathValue);
}

// -----------------------------
// Install dir discovery
// -----------------------------
static bool fileExists(const fs::path& p)
{
    std::error_code ec;
    return fs::exists(p, ec);
}

static fs::path tryFindInstallDirFromKnownLocations()
{
    std::wstring programFiles = getEnvVar(L"ProgramFiles");
    std::wstring localAppData = getEnvVar(L"LocalAppData");

    if (!programFiles.empty())
    {
        fs::path exe = fs::path(programFiles) / L"Neoluma" / L"bin" / L"Neoluma.exe";
        if (fileExists(exe)) return exe.parent_path().parent_path(); // .../Neoluma
    }

    if (!localAppData.empty())
    {
        fs::path exe = fs::path(localAppData) / L"Neoluma" / L"bin" / L"Neoluma.exe";
        if (fileExists(exe)) return exe.parent_path().parent_path();
    }

    return {};
}

static fs::path tryFindInstallDirFromPath()
{
    std::vector<std::wstring> allEntries;

    auto userEntries = getPathEntriesFromRegistry(HKEY_CURRENT_USER, L"Environment");
    allEntries.insert(allEntries.end(), userEntries.begin(), userEntries.end());

    auto systemEntries = getPathEntriesFromRegistry(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    allEntries.insert(allEntries.end(), systemEntries.begin(), systemEntries.end());

    for (const auto& e : allEntries)
    {
        std::wstring trimmed = trim(e);
        if (trimmed.empty()) continue;

        std::wstring norm = normalizePath(trimmed);

        if (norm.find(L"\\neoluma\\bin") == std::wstring::npos)
            continue;

        fs::path binDir = trimmed;
        fs::path exe = binDir / L"Neoluma.exe";
        if (fileExists(exe))
            return binDir.parent_path(); // .../Neoluma
    }

    return {};
}

static fs::path findInstallDir()
{
    fs::path a = tryFindInstallDirFromKnownLocations();
    if (!a.empty()) return a;

    fs::path b = tryFindInstallDirFromPath();
    if (!b.empty()) return b;

    return {};
}

// -----------------------------
// Registry cleanup (exact key name)
// -----------------------------
static void removeNeolumaRegistryExact()
{
    const std::wstring uninstallParent = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

    // Your installer registers here (user or system):
    // HKCU\Software\Microsoft\Windows\CurrentVersion\Uninstall\org.astrahelm.neoluma
    // HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\org.astrahelm.neoluma
    deleteRegKeyRecursive(HKEY_CURRENT_USER, uninstallParent, kUninstallId);
    deleteRegKeyRecursive(HKEY_LOCAL_MACHINE, uninstallParent, kUninstallId);

    // Optional: if some installer ever writes 32-bit view, try WOW6432Node too
    const std::wstring uninstallWowParent = L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    deleteRegKeyRecursive(HKEY_LOCAL_MACHINE, uninstallWowParent, kUninstallId);
}

// -----------------------------
// Cleanup worker spawn
// -----------------------------
static bool spawnCleanupWorker(const fs::path& tempExe, const fs::path& installDir)
{
    std::wstring cmd;
    cmd += L"\"";
    cmd += tempExe.wstring();
    cmd += L"\" --cleanup \"";
    cmd += installDir.wstring();
    cmd += L"\"";

    STARTUPINFOW si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> buf(cmd.begin(), cmd.end());
    buf.push_back(L'\0');

    BOOL ok = CreateProcessW(nullptr, buf.data(),
                             nullptr, nullptr, FALSE,
                             CREATE_NO_WINDOW,
                             nullptr, nullptr,
                             &si, &pi);
    if (!ok) return false;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

// -----------------------------
// Core cleanup logic
// -----------------------------
static void doCleanup(const fs::path& installDir)
{
    fs::path binDir = installDir / "bin";

    // PATH (user + system)
    removeEntryFromPath(HKEY_CURRENT_USER, L"Environment", binDir.wstring());
    removeEntryFromPath(HKEY_LOCAL_MACHINE,
                        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                        binDir.wstring());

    broadcastEnvChange();

    // Installed apps registry key (THIS now actually removes the key itself)
    removeNeolumaRegistryExact();
    broadcastUninstallChange();

    // Delete install directory
    std::error_code ec;
    fs::remove_all(installDir, ec);
}

// -----------------------------
// Entry point
// -----------------------------
int wmain(int argc, wchar_t** argv)
{
    // Always require admin (Program Files + system PATH + HKLM uninstall keys)
    if (!isRunningAsAdmin())
    {
        std::wcout << L"Neoluma Uninstaller needs administrator rights.\n";
        if (!relaunchAsAdmin(argc, argv))
        {
            std::wcout << L"Failed to request admin rights.\n";
            waitForEnter();
        }
        return 0;
    }

    // Cleanup mode (runs from temp copy)
    if (argc >= 3 && std::wstring(argv[1]) == L"--cleanup")
    {
        fs::path installDir = argv[2];

        std::wcout << L"Uninstalling Neoluma from:\n  " << installDir.wstring() << L"\n";
        doCleanup(installDir);

        // Best-effort: delete temp exe on reboot
        std::wstring selfPath = getSelfPath();
        if (!selfPath.empty())
            MoveFileExW(selfPath.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);

        std::wcout << L"Done.\n";
        return 0;
    }

    // Normal mode: locate install dir
    fs::path installDir = findInstallDir();
    if (installDir.empty())
    {
        std::wcout << L"Could not find Neoluma installation.\n";
        std::wcout << L"Checked:\n";
        std::wcout << L"  %ProgramFiles%\\Neoluma\\bin\\Neoluma.exe\n";
        std::wcout << L"  %LocalAppData%\\Neoluma\\bin\\Neoluma.exe\n";
        std::wcout << L"  User/System PATH entries containing \"Neoluma\\bin\"\n";
        waitForEnter();
        return 0;
    }

    // Copy uninstaller to temp and run cleanup from there
    std::wstring tempDir = getTempDir();
    if (tempDir.empty())
    {
        std::wcout << L"Could not get temp directory.\n";
        waitForEnter();
        return 0;
    }

    fs::path self = getSelfPath();
    if (self.empty())
    {
        std::wcout << L"Could not resolve current executable path.\n";
        waitForEnter();
        return 0;
    }

    fs::path tempExe = fs::path(tempDir) / L"neoluma_uninstall_tmp.exe";

    if (!CopyFileW(self.wstring().c_str(), tempExe.wstring().c_str(), FALSE))
    {
        std::wcout << L"Failed to copy uninstaller to temp:\n  " << tempExe.wstring() << L"\n";
        waitForEnter();
        return 0;
    }

    if (!spawnCleanupWorker(tempExe, installDir))
    {
        std::wcout << L"Failed to start cleanup worker.\n";
        waitForEnter();
        return 0;
    }

    // Exit immediately so worker can delete install dir even if this exe is inside it
    return 0;
}