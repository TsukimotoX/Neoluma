#include "Paths.hpp"
#include <stdexcept>

#if _WIN32
    #include <windows.h>
#elif __APPLE__ && __MACH__
    #include <mach-o/dyld.h>
    #include <limits.h>
    #include <unistd.h>
#else
    #include <limits.h>
    #include <unistd.h>
#endif

static std::string normalizeSlashes(std::string s){
    for (auto& c : s ) if (c == '\\') c = '/';
    return s;
}

std::string Paths::join(const std::string& a, const std::string& b){
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == '/' || a.back() == '\\') return a + b;
    return a + '/' + b;
}

std::string Paths::executablePath(){
#if _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) throw std::runtime_error("Paths::executablePath(): GetModuleFileNameA failed");
    return normalizeSlashes(std::string(buf, len));
#elif __APPLE__ && __MACH__
    uint32_t size = 0;
    _NSGetExecutablePath(NULL, &size);
    std::string out(size, '\0');
    if (_NSGetExecutablePath(out.data(), &size) != 0) throw std::runtime_error("Paths::executablePath(): _NSGetExecutablePath failed");
    out = std::string(out.c_str());
    char resolvedPath[PATH_MAX];
    if (realpath(out.c_str(), resolvedPath)) return std::string(resolvedPath);
    return out;
#else
    char resolvedPath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len <= 0) throw std::runtime_error("Paths::executablePath(): readlink failed");
    buf[len] = '\0';
    return std::string(buf);
#endif
}

std::string Paths::executableDir(){
    auto p = normalizeSlashes(executablePath());
    auto pos = p.find_last_of('/');
    if (pos == std::string::npos) return ".";
    return p.substr(0, pos);
}
