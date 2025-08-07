#pragma once

#include <iostream>
#include <variant>
#include <vector>
#include <map>

namespace Toml {
    struct TomlValue;

    using TomlArray = std::vector<TomlValue>;
    using TomlTable = std::map<std::string, TomlValue>;

    struct TomlValue {
        enum class Type { String, Boolean, Integer, Float, Array, Table, InlineTable } type;

        std::variant<std::string, int64_t, double, bool, TomlArray, TomlTable> value;

        TomlValue() = default;
        TomlValue(const std::string& str) : type(Type::String), value(str) {}
        TomlValue(const char* str) : type(Type::String), value(std::string(str)) {}
        TomlValue(bool b) : type(Type::Boolean), value(b) {}
        TomlValue(int64_t i) : type(Type::Integer), value(i) {}
        TomlValue(double f) : type(Type::Float), value(f) {}
        TomlValue(const TomlArray& arr) : type(Type::Array), value(arr) {}
        TomlValue(const TomlTable& tbl) : type(Type::Table), value(tbl) {}
    };

    inline TomlArray Array(std::initializer_list<TomlValue> list) {
        return TomlArray(list);
    }

    struct Table {
        TomlTable data;

        static Table make(const std::string& name) {
            return Table{};
        }

        void insert(const std::string& key, const TomlValue& val) {
            data[key] = val;
        }

        TomlTable& get() {
            return data;
        }
    };
}
