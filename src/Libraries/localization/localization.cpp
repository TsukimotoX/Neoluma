#include "localization.hpp"
#include <filesystem>
#include <fstream>

#if _WIN32
#include <Windows.h>
#endif

namespace Localization {
	static std::string detectSystemLanguage() {
		#if _WIN32 // i hate microsoft
			WCHAR buffer[LOCALE_NAME_MAX_LENGTH];
			int r = GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
			if (r > 0) {
				int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
				std::string out(len, 0);
                WideCharToMultiByte(CP_UTF8, 0, buffer, -1, out.data(), len, nullptr, nullptr);
				out.resize(len - 1);

                for (auto& c : out) if (c == '-') c = '_';
				return out;
			}
			return "en_US"; // fallback
		#else
			std::string lang = getenv("LANG");
			if (!lang) return "en_US";

			std::string s = lang;
			if (s.find('.') == std::string::npos) s = s.substr(0, s.find('.'));
			return s;
		#endif
	}

	static nlohmann::json loadJson(const std::string& locale) {
		std::filesystem::path path = std::filesystem::current_path() / "src" / "Localization" / (locale + ".json");
		if (!std::filesystem::exists(path)){
			std::cout << "[Localization] File does not exist: " << path << std::endl;
			return nlohmann::json::object();
		}

		std::ifstream file(path);
		nlohmann::json json;
		file >> json;
		file.close();
		return json;
	}

	void init() {
		fallbackJson = loadJson("en_US");
		fallbackMap.clear();
		pancakeJson(fallbackJson, "", fallbackMap);

		std::string sys = detectSystemLanguage();
		currentLocale = sys;
		localeMap.clear();
		nlohmann::json j = loadJson(sys);
		pancakeJson(j, "", localeMap);
	}

	void pancakeJson(const nlohmann::json& j, const std::string& prefix, std::unordered_map<std::string, std::string>& out) {
		if (j.is_object())
		{
			for (auto& item : j.items()) {
				std::string key = prefix.empty() ? item.key() : prefix + "." + item.key();
				pancakeJson(item.value(), key, out);
			}
		}
		else out[prefix] = j.is_string() ? j.get<std::string>() : j.dump();
	}

	void setLanguage(const std::string &language) {
		currentLocale = language;
		localeMap.clear();
		nlohmann::json j = loadJson(currentLocale);
		pancakeJson(j, "", localeMap);
	}

	std::string translate(const std::string& key) {
		auto item = localeMap.find(key);
		if (item != localeMap.end()) return item->second;

		auto fallbackItem = fallbackMap.find(key);
		if (fallbackItem != fallbackMap.end()) return fallbackItem->second;

		std::cerr << "[Localization] Missing key: " << key << "\n";
		return key;
	}
};