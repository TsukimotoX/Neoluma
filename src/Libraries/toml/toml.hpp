#pragma once

#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <string>
#include <sstream>

namespace Toml {
    struct TomlValue;

    using TomlArray = std::vector<TomlValue>;
    using TomlTable = std::vector<std::pair<std::string, TomlValue>>;

    struct TomlValue {
        enum class Type { String, Boolean, Integer, Float, Array, Table } type;

        std::variant<std::string, int64_t, double, bool, TomlArray, TomlTable> value;

        TomlValue() = default;
        TomlValue(const std::string& str) : type(Type::String), value(str) {}
        TomlValue(const char* str) : type(Type::String), value(std::string(str)) {}

        TomlValue(bool b) : type(Type::Boolean), value(b) {}
        TomlValue(int64_t i) : type(Type::Integer), value(i) {}
        TomlValue(double f) : type(Type::Float), value(f) {}
        TomlValue(const TomlArray& arr) : type(Type::Array), value(arr) {}
        TomlValue(const TomlTable& tbl) : type(Type::Table), value(tbl) {}
        
        TomlValue& operator[](const std::string& key);

    };

    inline TomlArray Array(std::initializer_list<TomlValue> list);

    struct Table {
        TomlTable data;
        static Table make(const std::string& name);
        TomlValue& operator[](const std::string& key);
        const TomlValue& operator[](const std::string& key) const;
        TomlTable& get();
    };

    std::string serialize(const TomlValue& value);

    void serializeTable(std::ostream& out, Table& table, const std::string& parent = "");

    TomlValue parseValue(const std::string& value);

    Table parseToml(std::istream& in);
}
