#include "errormanager.hpp"
#include <print>

void ErrorManager::printErrors(const std::string& source) {
    for (auto& e : errors) {
        /*
        ❌ [{type}] : {message}
            ➡️ {path}:{line}:{column}
            |
     {line} | {context} 
            |    ^ {errorMsg}
        💡 {hintkw}: {hint}
        */
    }
}