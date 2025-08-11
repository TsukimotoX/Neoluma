#include "toml.hpp"

namespace Toml {
    std::string escapeString(const std::string& str) {
        std::string res;
        for (char c : str) {
            if (c == '\\' || c == '"') {
                res += '\\';
            }
            res += c;
        }
        return res;
    }

    std::string trimstr(const std::string& s) {
        size_t start = 0;
        size_t end = s.size() - 1;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        if (start == s.size()) return "";
        while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
        return s.substr(start, end - start + 1);
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    Table& getOrCreateTable(Table& root, const std::vector<std::string>& path) {
        Table* current = &root;
        for (const auto& p : path) {
            auto& val = (*current)[p];
            if (val.type != TomlValue::Type::Table) val = TomlValue(Table{}.get());
            TomlTable& inner = std::get<TomlTable>(val.value);
            current = new Table{inner};
        }
        return *current;
    }

    TomlValue& TomlValue::operator[](const std::string& key){
        if (type != Type::Table) {
            type = Type::Table;
            value = TomlTable{};
        }
        auto& table = std::get<TomlTable>(value);
        for (auto& [k, v] : table) {
            if (k == key) return v;
        }
        table.emplace_back(key, TomlValue{});
        return table.back().second;
    }

    inline TomlArray Array(std::initializer_list<TomlValue> list) { return TomlArray(list); }

    Table Table::make(const std::string& name) { return Table{}; }
    TomlValue& Table::operator[](const std::string& key){ 
        for (auto& [k, v] : data) {
            if (k == key) return v;
        }
        data.emplace_back(key, TomlValue{});
        return data.back().second;
    }
    const TomlValue& Table::operator[](const std::string& key) const {
        for (const auto& [k, v] : data) {
            if (k == key) return v;
        }
        throw std::out_of_range("[NeolumaLibs/toml] Key '" + key + "' not found in Table");
    }
    TomlTable& Table::get() { return data; }

    std::string serialize(const TomlValue& value) {
        using Type = TomlValue::Type;
        switch (value.type) {
            case Type::String:
                return "\"" + escapeString(std::get<std::string>(value.value)) + "\"";
            case Type::Boolean:
                return std::get<bool>(value.value) ? "true" : "false";
            case Type::Integer:
                return std::to_string(std::get<int64_t>(value.value));
            case Type::Float:
                return std::to_string(std::get<double>(value.value));
            case Type::Array: {
                const auto& arr = std::get<TomlArray>(value.value);
                std::string s = "[";
                for (size_t i = 0; i < arr.size(); ++i) {
                    s += serialize(arr[i]);
                    if (i + 1 < arr.size()) s += ", ";
                }
                s += "]";
                return s;
            }
            case Type::Table: {
                std::ostringstream out;
                Toml::Table t(std::get<TomlTable>(value.value));
                serializeTable(out, t);
                return out.str();
            }
            default:
                return "";
        }
    }

    void serializeTable(std::ostream& out, Table& table, const std::string& parent) {
        for (const auto& [key, value] : table.data) {
            if (value.type != TomlValue::Type::Table) {
                out << key << " = " << serialize(value) << "\n";
            }
        }

        for (const auto& [key, value] : table.data) {
            if (value.type == TomlValue::Type::Table) {
                std::string fullKey = parent.empty() ? key : parent + "." + key;
                out << "\n[" << fullKey << "]\n";
                Table t(std::get<TomlTable>(value.value));
                serializeTable(out, t, fullKey);
            }
        }
    }

    TomlValue parseValue(const std::string& value) {
        std::string trimmed = trimstr(value);
        if (trimmed.empty()) return TomlValue();

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            std::string inside = trimmed.substr(1, trimmed.size() - 2);
            TomlArray arr;

            std::istringstream ss(inside);
            std::string item;
            while (std::getline(ss, item, ',')) {
                arr.push_back(parseValue(trimstr(item)));
            }
            return TomlValue(arr);
        }

        if (trimmed.front() == '"' && trimmed.back() == '"') {
            return TomlValue(trimmed.substr(1, trimmed.size() - 2));
        }

        if (trimmed == "true") return TomlValue(true);
        if (trimmed == "false") return TomlValue(false);

        try {
            size_t pos;
            int64_t i = std::stoll(trimmed, &pos);
            if (pos == trimmed.size()) return TomlValue(i);
        } catch (...) {}

        try {
            size_t pos;
            double d = std::stod(trimmed, &pos);
            if (pos == trimmed.size()) return TomlValue(d);
        } catch (...) {}

        return TomlValue(trimmed);
    }

    Table parseToml(std::istream& in) {
        Table root;
        Table* currentTable = &root;

        std::string line;
        while (std::getline(in, line)) {
            size_t comment_pos = line.find('#');
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
            line = trimstr(line);

            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string tableName = line.substr(1, line.size() - 2);
                auto parts = split(tableName, '.');
                currentTable = &getOrCreateTable(root, parts);
            }
            else {
                size_t eq_pos = line.find('=');
                if (eq_pos == std::string::npos) continue;

                std::string key = trimstr(line.substr(0, eq_pos));
                std::string val = trimstr(line.substr(eq_pos + 1));

                (*currentTable)[key] = parseValue(val);
            }
        }

        return root;
    }
}

