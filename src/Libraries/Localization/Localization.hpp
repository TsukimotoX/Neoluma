#pragma once
/*
* Localization is an internal Neoluma library for handling localizations of the internal docs / cli / errors / etc.
*/

#include <iostream>
#include "../json/json.hpp"
#include <unordered_map>

namespace Localization {
	static std::unordered_map<std::string, std::string> fallbackMap;
	static std::unordered_map<std::string, std::string> localeMap;
	// TODO: Temporary implementation, when we save files on installed path it must be changed to check those
	static nlohmann::json fallbackJson; // en_US language;
	static std::string currentLocale = "en_US";

	static std::string detectSystemLanguage();
	static nlohmann::json loadJson(const std::string& locale);
	void init();
	void pancakeJson(const nlohmann::json& j, const std::string& prefix, std::unordered_map<std::string, std::string>& out);
	std::string translate(const std::string& key);
	void setLanguage(const std::string& language); // Manual language override
};