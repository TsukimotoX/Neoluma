#pragma once

#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <string>
#include <sstream>

namespace Toml {
    enum class TomlType { Comment, String, Integer, Float, Boolean, Array, Table };

    struct TomlValue;
    
    using TomlArray = std::vector<TomlValue>;
    using TomlTable = std::map<std::string, TomlValue>;
    
    struct TomlValue {
        TomlType type;
        std::variant<std::string, int64_t, double, bool, TomlArray, TomlTable> value;
        TomlValue() = default;
        TomlValue(const std::string& str) : type(TomlType::String), value(str) {}
        TomlValue(bool b) : type(TomlType::Boolean), value(b) {}
        TomlValue(int64_t i) : type(TomlType::Integer), value(i) {}
        TomlValue(double f) : type(TomlType::Float), value(f) {}
        TomlValue(const TomlArray& arr) : type(TomlType::Array), value(arr) {}
        TomlValue(const TomlTable& tbl) : type(TomlType::Table), value(tbl) {}

        TomlValue& operator[](const std::string& key);
        const TomlValue& operator[](const std::string& key) const;
    };
    /*
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
    */
};
