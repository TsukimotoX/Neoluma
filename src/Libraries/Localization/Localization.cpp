#include "Localization.hpp"

#include <filesystem>
#include <print>
#include <iostream>
#if _WIN32
    #include <Windows.h>
#else
    #include <cstdlib>
#endif

namespace Localization {

    // ---- Globals (defined once here) ----
    std::unordered_map<std::string, std::string> fallbackMap;
    std::unordered_map<std::string, std::string> localeMap;

    json::Value fallbackJson = json::Value(json::Object{});
    std::string currentLocale = "en_US";

    // ---- Internal: stringify non-string values in a stable way ----
    static std::string valueToString(const json::Value& v) {
        if (v.isString()) return v.asString();

        // When flattening, keep it compact + no comments
        json::StringifyOptions opt;
        opt.pretty = false;
        opt.emit_comments = false;
        opt.escape_non_ascii = false;
        opt.sort_keys = false;
        return json::stringify(v, opt);
    }

    std::string detectSystemLanguage() {
    #if _WIN32 // i hate microsoft
        ULONG numLangs = 0;
        ULONG bufferLength = 0;

        GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLangs, nullptr, &bufferLength);
        if (bufferLength > 0) {
            std::wstring buffer(bufferLength, L'\0');
            if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLangs, buffer.data(), &bufferLength)) {
                std::wstring firstLang = buffer.data(); // first system language

                int len = WideCharToMultiByte(CP_UTF8, 0, firstLang.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string out((size_t)len, 0);
                WideCharToMultiByte(CP_UTF8, 0, firstLang.c_str(), -1, out.data(), len, nullptr, nullptr);
                out.resize((size_t)len - 1);

                for (auto& c : out) if (c == '-') c = '_';
                return out;
            }
        }

        // old method fallback just in case
        WCHAR buffer[LOCALE_NAME_MAX_LENGTH];
        int r = GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
        if (r > 0) {
            int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
            std::string out((size_t)len, 0);
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, out.data(), len, nullptr, nullptr);
            out.resize((size_t)len - 1);

            for (auto& c : out) if (c == '-') c = '_';
            return out;
        }

        return "en_US"; // fallback
    #else
        const char* langPtr = std::getenv("LANG");
        if (!langPtr) return "en_US";

        std::string s = langPtr;

        // LANG often: "en_US.UTF-8"
        if (auto dot = s.find('.'); dot != std::string::npos)
            s = s.substr(0, dot);

        // Sometimes it can be "en_US@modifier"
        if (auto at = s.find('@'); at != std::string::npos)
            s = s.substr(0, at);

        return s;
    #endif
    }

    json::Value loadJson(const std::string& locale) {
        // Keep your original layout
        // TODO on return: Make multipaths support of json for convenience.
        std::filesystem::path path =
            std::filesystem::current_path() / "src" / "Localization" / (locale + ".jsonc");

        if (!std::filesystem::exists(path)) {
            std::println(std::cerr, "[Localization] File does not exist: {}", path.string());
            return json::Value(json::Object{});
        }

        try {
            json::Value root = json::parseFile(path.string());

            if (!root.isObject()) {
                std::println(std::cerr, "[Localization] JSON root is not an object in: {}", path.string());
                return json::Value(json::Object{});
            }

            return root;
        }
        catch (const json::ParseError& e) {
            std::println(
                std::cerr,
                "[Localization] JSON parse error in {} at line {}, col {}: {}",
                path.string(),
                e.line, e.col,
                e.what()
            );
            return json::Value(json::Object{});
        }
        catch (const std::exception& e) {
            std::println(std::cerr, "[Localization] Failed to read {}: {}", path.string(), e.what());
            return json::Value(json::Object{});
        }
    }

    void pancakeJson(const json::Value& v,
                     const std::string& prefix,
                     std::unordered_map<std::string, std::string>& out) {

        if (v.isObject()) {
            for (const auto& [k, child] : v.asObject()) {
                std::string key = prefix.empty() ? k : (prefix + "." + k);
                pancakeJson(child, key, out);
            }
            return;
        }

        // Arrays are valid too — flatten them as JSON text
        out[prefix] = valueToString(v);
    }

    void init() {
        // Load fallback first
        fallbackJson = loadJson("en_US");
        fallbackMap.clear();
        pancakeJson(fallbackJson, "", fallbackMap);

        // Load system locale
        std::string sys = detectSystemLanguage();
        currentLocale = sys;

        localeMap.clear();
        json::Value j = loadJson(sys);
        pancakeJson(j, "", localeMap);
    }

    void setLanguage(const std::string& language) {
        currentLocale = language;
        localeMap.clear();
        json::Value j = loadJson(currentLocale);
        pancakeJson(j, "", localeMap);
    }

    std::string translate(const std::string& key) {
        if (auto it = localeMap.find(key); it != localeMap.end())
            return it->second;

        std::println(std::cerr, "[Localization] Couldn't translate key '{}'", key);

        if (auto fb = fallbackMap.find(key); fb != fallbackMap.end())
            return fb->second;

        std::println(std::cerr, "[Localization] Missing key: '{}'", key);
        return key;
    }

} // namespace Localization
