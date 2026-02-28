#pragma once
/*
* Localization is an internal Neoluma library for handling localizations of the internal docs / cli / errors / etc.
*/

#include <string>
#include <unordered_map>

#include "../Json/Json.hpp" // new json lib

/*
 * Goals to remake Localization:
 * - Allow to address via keys (safely). Like: localeMap["ErrorManager.Syntax.UnexpectedToken.message"]
 * - Save localization to path where the compiler is executed.
 * - Make locale compare to English and combine into one hashmap if there's missing keys (for less memory overhead)
 * - Detect language once. After that make it changeable through CLI or config.
 */

namespace Localization {
	// Runtime keys storage
	extern std::unordered_map<std::string, std::string> localeMap;

	extern json::Value fallbackJson;     // en_US language root
	extern std::string currentLocale;    // current locale id

	// Internal helpers
	std::string detectSystemLanguage(); // runs only once if configs don't exist
	json::Value loadJson(const std::string& locale);

	void init();

	void pancakeJson(
		const json::Value& v,
		const std::string& prefix,
		std::unordered_map<std::string, std::string>& out
	);

	std::string translate(const std::string& key);
	void setLanguage(const std::string& language); // Manual language override
}
