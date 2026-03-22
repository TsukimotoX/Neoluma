#include "ProjectManager.hpp"

std::string ProjectManager::normalizeSlashes(std::string s) {
    for (auto& c : s) if (c == '\\') c = '/';
    return s;
}

bool ProjectManager::endsWith(const std::string& s, const std::string& suf) {
    return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

std::string ProjectManager::filePathToKey(const std::string& filePath) {
    std::string s = normalizeSlashes(filePath);

    auto pos = s.rfind(config.sourceFolder);
    if (pos != std::string::npos) s = s.substr(pos+config.sourceFolder.length());

    if (endsWith(s, ".nm")) s = s.substr(0, s.size() - 3);

    return s;
}
