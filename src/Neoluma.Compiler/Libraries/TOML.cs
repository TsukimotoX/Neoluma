/*
   TOML is an internal Neoluma library responsible for reading and writing TOML data.
   It is primarily used for Neoluma project files, but can also be exposed through std later.
  
   ====== Quick usage ======
  
   1) Parse TOML
   -------------
   TOML.Table root = TOML.parse(text);
   // or
   TOML.Table root = TOML.parseFile("project.nlp");
  
   > Throws TOML.ParseError on invalid input.
  
   2) Read values
   --------------
   string name = root["project"]["name"].asString();
   string version = root["project"]["version"].asString();
  
   bool debug = root["settings"]["debug"].asBool();
   long threads = root["settings"]["threads"].asInt();
  
   TOML.Array authors = root["project"]["authors"].asArray();
  
   > Values can be checked before reading:
  
   if (root["project"]["name"].isString()) {
       string name = root["project"]["name"].asString();
   }
  
   > Available checks:
  
   isString()
   isInt()
   isDouble()
   isBool()
   isArray()
   isTable()
   isLocalDate()
   isLocalTime()
   isLocalDateTime()
   isOffsetDateTime()
  
   3) Modify values
   ----------------
   root["project"]["name"] = "My Project";
   root["project"]["version"] = "0.2.0";
   root["settings"]["debug"] = true;
   root["settings"]["threads"] = 4;
  
   4) Create tables
   ----------------
   TOML.Table project = [
       ("name", "Neoluma"),
       ("version", "0.1.0"),
       ("license", "mit"),
       ("output", "exe")
   ];
  
   TOML.Table dependencies = [
       ("std", "0.1.0")
   ];
  
   TOML.Table root = [
       ("project", project),
       ("dependencies", dependencies)
   ];
  
   5) Create arrays
   ----------------
   TOML.Array authors = [
       "TsukimotoX",
       "Someone else"
   ];
  
   root["project"]["authors"] = authors;
  
   6) Write TOML
   -------------
   string text = TOML.stringify(root);
   // or
   TOML.writeFile("project.nlp", root);
  
   Notes
   -----
   - The root of a TOML document is always TOML.Table.
   - Table values can be accessed recursively using ["key"].
   - Value supports implicit conversion from common C# types.
   - stringify() produces canonical TOML and does not preserve the original formatting.
   - Comments from the source are not preserved.
   - Duplicate keys and invalid table redefinitions are rejected.
  
   Supported TOML features:
   - bare, quoted and dotted keys
   - basic and literal strings
   - multiline strings
   - integers, floats, inf and nan
   - booleans
   - arrays
   - inline tables
   - normal tables: [table]
   - arrays of tables: [[table]]
   - local date/time/date-time
   - offset date-time
   - comments
*/

using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace Neoluma.Libraries;

public static class TOML {
    // ====== Error ======
    public class ParseError(string message, int line, int column)
        : Exception($"{message} at line {line}, column {column}") {
        public int line { get; } = line;
        public int column { get;  } = column;
    }
    
    // ====== Types ======
    public class Property(string key, Value value) {
        public string key = key;
        public Value value = value;
        
        public static implicit operator Property((string key, Value value) property) {
            return new(property.key, property.value);
        }
    }
    public class Array : List<Value> { }
    public class Table : List<Property> {
        public Value this[string key] {
            get {
                foreach (var property in this) 
                    if (property.key == key) return property.value;

                throw new KeyNotFoundException($"[Neoluma/TOML] TOML key '{key}' not found.");
            }

            set {
                foreach(var property in this) 
                    if  (property.key == key) {
                        property.value = value;
                        return;
                    }
                
                Add(new Property(key, value));
            }
        }

        public bool tryGet(string key, out Value? value) {
            foreach (var property in this)
                if (property.key == key) {
                    value = property.value;
                    return true;
                }
            
            value = null;
            return false;
        }
        
    }
    public class Value {
        private object? storage;

        public Value() { }
        public Value(string value) => storage = value;
        public Value(long value) => storage = value;
        public Value(double value) => storage = value;
        public Value(bool value) => storage = value;
        public Value(Array value) => storage = value;
        public Value(Table value) => storage = value;
        public Value(DateOnly value) => storage = value;
        public Value(TimeOnly value) => storage = value;
        public Value(DateTime value) => storage = value;
        public Value(DateTimeOffset value) => storage = value;

        public bool isString() => storage is string;
        public bool isInt() => storage is long;
        public bool isDouble() => storage is double;
        public bool isBool() => storage is bool;
        public bool isArray() => storage is Array;
        public bool isTable() => storage is Table;
        public bool isLocalDate() => storage is DateOnly;
        public bool isLocalTime() => storage is TimeOnly;
        public bool isLocalDateTime() => storage is DateTime;
        public bool isOffsetDateTime() => storage is DateTimeOffset;

        public string asString() => storage is string v ? v : "";
        public long asInt() => storage is long v ? v : 0;
        public double asDouble() {
            if (storage is double v) return v;
            if (storage is long i) return i;
            return 0;
        }
        public bool asBool() => storage is bool v && v;
        public Array asArray() => storage is Array v ? v : [];
        public Table asTable() => storage is Table v ? v : [];
        public DateOnly asLocalDate() => storage is DateOnly v ? v : default;
        public TimeOnly asLocalTime() => storage is TimeOnly v ? v : default;
        public DateTime asLocalDateTime() => storage is DateTime v ? v : default;
        public DateTimeOffset asOffsetDateTime() => storage is DateTimeOffset v ? v : default;
        
        public object? raw() => storage;

        public Value this[string key] {
            get => storage is Table table ? table[key] : throw new InvalidOperationException("[Neoluma/TOML] TOML value is not a table");
            set {
                if (storage is not Table table) throw new InvalidOperationException("[Neoluma/TOML] TOML value is not a table");
                table[key] = value;
            }
        }

        public static implicit operator Value(string v) => new(v);
        public static implicit operator Value(long v) => new(v);
        public static implicit operator Value(int v) => new(v);
        public static implicit operator Value(double v) => new(v);
        public static implicit operator Value(bool v) => new(v);
        public static implicit operator Value(Array v) => new(v);
        public static implicit operator Value(Table v) => new(v);
    }
    
    // ====== Main API ======
    public static Table parse(string text) {
        Parser parser = new();
        return parser.parse(text);
    }
    public static Table parseFile(string filePath) => parse(File.ReadAllText(filePath));
    public static string stringify(Table table) {    
        Stringifier stringifier = new();
        return stringifier.stringify(table); 
    }
    public static void writeFile(string path, Table table) => File.WriteAllText(path, stringify(table));

    private sealed class Parser {
        private string source = "";
        private int pos;
        private int line = 1;
        private int column = 1;

        private Table root = null!;
        private Table currentTable = null!;

        // distinguish normal arrays from arrays of tables
        private readonly HashSet<Array> arrayTables = [];
        // declared [tables] 
        private readonly HashSet<Table> declaredTables = [];
        // inline tables (can't be extended later)
        private readonly HashSet<Table> inlineTables = [];

        // ====== Helpers ======
        char peek(int offset = 0) {
            int index = pos + offset;
            return index < source.Length ? source[index] : '\0';
        }
        char move() {
            if (isAtEnd()) return '\0';

            char c = source[pos++];
            if (c == '\n') {
                line++;
                column = 1;
            }
            else column++;

            return c;
        }
        bool isAtEnd() => pos >= source.Length;
        bool match(string text) {
            if (pos + text.Length > source.Length) return false;
            return source.AsSpan(pos, text.Length).SequenceEqual(text);
        }
        void expect(char expected) {
            if (peek() != expected) throw error($"Expected '{expected}', got '{peek()}'");
            move();
        }
        void expect(string expected) {
            if (!match(expected)) throw error($"Expected '{expected}'");
            
            for (int i = 0; i < expected.Length; i++) move();
        }
        void skipWhitespaces() {
            while (peek() == ' ' || peek() == '\t') move();
        }
        void skipComment() {
            if (peek() != '#') return;
            while (!isAtEnd() && peek() != '\n') move();
        }
        int countAhead(char value) {
            int count = 0;
            while (peek(count) == value) count++;
            return count;
        }
        ParseError error(string message) => new(message, line, column);

        public Table parse(string source) {
            this.source = source.Replace("\r\n", "\n").Replace('\r', '\n');
            pos = 0; line = 1; column = 1;

            root = new Table();
            currentTable = root;
            
            arrayTables.Clear(); declaredTables.Clear(); inlineTables.Clear();
            
            // get rid of utf-8 bom
            if (!isAtEnd() && peek() == '\uFEFF') move();

            while (!isAtEnd()) {
                skipWhitespaces();

                if (isAtEnd()) break;
                if (peek() == '\n') { move(); continue; }
                if (peek() == '#') { skipComment(); continue; }

                if (peek() == '[') parseTableHeader();
                else parseKeyValue(currentTable, true);
            }

            return root;
        }

        void parseKeyValue(Table table, bool requireNewline) {
            List<string> path = parseKey();
            
            skipWhitespaces();
            expect('=');
            skipWhitespaces();

            Value value = parseValue();
            setValue(table, path, value);
            
            if (!requireNewline) return;
            skipWhitespaces();
            
            if (peek() == '#') skipComment();
            if (!isAtEnd() && peek() != '\n') throw error("Expected end of line");
            if (!isAtEnd()) move();
        }

        void parseTableHeader() {
            expect('[');

            bool isArrayTable = false;
            if (peek() == '[') {
                isArrayTable = true;
                move();
            }
            
            skipWhitespaces();
            List<string> path = parseKey();
            skipWhitespaces();
            
            expect(']');
            if (isArrayTable) expect(']');
            skipWhitespaces();
            if (peek() == '#') skipComment();
            if (!isAtEnd() && peek() != '\n') throw error("Expected end of line after table declaration");
            if (!isAtEnd()) move();
            if (isArrayTable) currentTable = createArrayTable(path);
            else currentTable = createTable(path);
        }

        Table createTable(List<string> path) {
            if (path.Count == 0) throw error("Table name cannot be empty");

            Table table = root;

            foreach (string part in path) {
                if (inlineTables.Contains(table)) throw error("Inline table cannot be extended");

                if (!table.tryGet(part, out Value? existing)) {
                    Table child = new();
                    table.Add(new Property(part, child));
                    table = child;
                    continue;
                }
                
                if (existing!.isTable()) {
                    table = existing.asTable();
                    continue;
                }
                
                // [x.y] after [[x]]
                if (existing.isArray()) {
                    Array array = existing.asArray();

                    if (!arrayTables.Contains(array) || array.Count == 0 || !array[^1].isTable())
                        throw error($"'{part} is not a table");
                    
                    table = array[^1].asTable();
                    continue;
                }

                throw error($"'{part}' is already defined as a value");
            }
            
            if (declaredTables.Contains(table)) throw error("Table is already defined");
            
            if (inlineTables.Contains(table)) throw error("Inline table cannot be redefined");
            
            declaredTables.Add(table);
            return table;
        }

        Table createArrayTable(List<string> path) {
            if (path.Count == 0) throw error("Array table name cannot be empty");

            Table parent = resolveParentTable(root, path);
            string name = path[^1];
            Array array;

            if (!parent.tryGet(name, out Value? existing)) {
                array = new Array();
                arrayTables.Add(array);
                parent.Add(new Property(name, array));
            }
            else {
                if (!existing!.isArray()) 
                    throw error($"'{name}' is already defined and is not an array");
                
                array = existing.asArray();

                if (!arrayTables.Contains(array)) 
                    throw error($"'{name}' is already defined as a normal array");
            }

            Table table = new();
            array.Add(table);
            
            declaredTables.Add(table);
            return table;
        }
        
        // ====== Keys ======
        List<string> parseKey() {
            List<string> path = [];

            while (true) {
                skipWhitespaces();
                
                path.Add(parseKeyPart());
                skipWhitespaces();

                if (peek() != '.') break;
                move();
            }

            return path;
        }

        string parseKeyPart() {
            if (peek() == '"') return parseBasicString(false);
            if (peek() == '\'') return parseLiteralString(false);
            StringBuilder result = new();

            while (!isAtEnd()) {
                char c = peek();
                if (!(char.IsAsciiLetterOrDigit(c) || c == '_' || c == '-')) break;
                result.Append(move());
            }

            if (result.Length == 0) throw error("Expected TOML key");
            return result.ToString();
        }
        
        // ====== Values ======
        Value parseValue() {
            if (isAtEnd()) throw error("Expected value");

            if (match("\"\"\"")) return parseBasicString(true);
            if (match("'''")) return parseLiteralString(true);
            if (peek() == '"') return parseBasicString(false);
            if (peek() == '\'') return parseLiteralString(false);
            if (peek() == '[') return parseArray();
            if (peek() == '{') return parseInlineTable();
            return parseBareValue();
        }

        Value parseBareValue() {
            string raw = readBareValue();

            if (raw.Length == 0) throw error("Expected value");
            if (raw == "true") return true;
            if (raw == "false") return false;
            if (tryParseDateTime(raw, out Value? dateTime)) return dateTime!;
            if (tryParseInteger(raw, out long integer)) return integer;
            if (tryParseFloat(raw, out double floating)) return floating;

            throw error($"Invalid TOML value '{raw}'");
        }

        string readBareValue() {
            StringBuilder result = new();

            while (!isAtEnd()) {
                char c = peek();

                if (c == ',' || c == ']' || c == '}' || c == '#' || c == '\n' || c == '\t') break;
                if (c == ' ') {
                    // TOML allows a space instead of T in datetime:
                    // 1979-05-27 07:32:00Z
                    if (result.Length == 10 &&
                        Regex.IsMatch(result.ToString(), @"^\d{4}-\d{2}-\d{2}$") &&
                        char.IsDigit(peek(1))) {
                        result.Append(move());
                        continue;
                    }

                    break;
                }

                result.Append(move());
            }

            return result.ToString();
        }

        // ====== Strings ======

        string parseBasicString(bool multiline) {
            if (multiline) {
                expect("\"\"\"");
                if (peek() == '\n') move();
            } else expect('"');

            StringBuilder result = new();

            while (!isAtEnd()) {
                if (multiline && peek() == '"') {
                    int quotes = countAhead('"');

                    if (quotes >= 3) {
                        for (int i = 0; i < quotes; i++) move();
                        
                        // """" -> one literal quote + close
                        // """"" -> two literal quotes + close
                        for (int i = 0; i < quotes - 3; i++) 
                            result.Append('"');

                        return result.ToString();
                    }
                }
                if (!multiline && peek() == '"') {
                    move();
                    return result.ToString();
                }
                if (peek() == '\n') {
                    if (!multiline)
                        throw error("Basic string cannot contain a newline");

                    result.Append(move());
                    continue;
                }
                if (peek() == '\\') {
                    move();

                    if (multiline && tryConsumeStringContinuation())
                        continue;

                    result.Append(parseEscape());
                    continue;
                }

                char c = move();
                if (c < 0x20 && c != '\t')
                    throw error("Control character is not allowed in string");
                result.Append(c);
            }

            throw error("Unterminated string");
        }

        string parseLiteralString(bool multiline) {
            if (multiline) {
                expect("'''");
                if (peek() == '\n') move();
            } else expect('\'');

            StringBuilder result = new();

            while (!isAtEnd()) {
                if (multiline && peek() == '\'') {
                    int quotes = countAhead('\'');

                    if (quotes >= 3) {
                        for (int i = 0; i < quotes; i++)
                            move();

                        for (int i = 0; i < quotes - 3; i++)
                            result.Append('\'');

                        return result.ToString();
                    }
                }

                if (!multiline && peek() == '\'') {
                    move();
                    return result.ToString();
                }

                if (peek() == '\n') {
                    if (!multiline)
                        throw error("Literal string cannot contain a newline");

                    result.Append(move());
                    continue;
                }
                
                char c = move();
                if (c < 0x20 && c != '\t')
                    throw error("Control character is not allowed in string");
                result.Append(c);
            }

            throw error("Unterminated literal string");
        }

        bool tryConsumeStringContinuation() {
            int offset = 0;

            while (peek(offset) == ' ' || peek(offset) == '\t') offset++;
            if (peek(offset) != '\n') return false;
            while (peek() == ' ' || peek() == '\t') move();

            move(); // newline

            while (!isAtEnd()) {
                char c = peek();

                if (c == ' ' || c == '\t' || c == '\n') move();
                else break;
            }

            return true;
        }

        string parseEscape() {
            if (isAtEnd())
                throw error("String ends after escape character");

            char escaped = move();

            return escaped switch {
                'b' => "\b",
                't' => "\t",
                'n' => "\n",
                'f' => "\f",
                'r' => "\r",
                '"' => "\"",
                '\\' => "\\",
                'u' => parseUnicodeEscape(4),
                'U' => parseUnicodeEscape(8),
                _ => throw error($"Invalid escape sequence '\\{escaped}'")
            };
        }

        string parseUnicodeEscape(int length) {
            StringBuilder hex = new();

            for (int i = 0; i < length; i++) {
                if (isAtEnd() || !char.IsAsciiHexDigit(peek()))
                    throw error($"Expected {length} hexadecimal digits");

                hex.Append(move());
            }

            int value;
            try {
                value = Convert.ToInt32(hex.ToString(), 16);
            } catch {
                throw error("Invalid Unicode escape");
            }

            if (value > 0x10FFFF || value is >= 0xD800 and <= 0xDFFF)
                throw error("Invalid Unicode code point");

            return char.ConvertFromUtf32(value);
        }

        // ====== Arrays ======

        Value parseArray() {
            expect('[');

            Array array = new();

            skipArrayWhitespace();

            if (peek() == ']') {
                move();
                return array;
            }

            while (!isAtEnd()) {
                array.Add(parseValue());
                skipArrayWhitespace();

                if (peek() == ']') { move(); return array; }
                if (peek() != ',')
                    throw error("Expected ',' or ']' in array");

                move();
                skipArrayWhitespace();

                // Trailing comma is valid in arrays.
                if (peek() == ']') { move(); return array; }
            }

            throw error("Unterminated array");
        }

        void skipArrayWhitespace() {
            while (!isAtEnd()) {
                if (peek() == ' ' || peek() == '\t' || peek() == '\n') {
                    move();
                    continue;
                }

                if (peek() == '#') {
                    skipComment();
                    continue;
                }

                break;
            }
        }

        // ====== Inline tables ======

        Value parseInlineTable() {
            expect('{');

            Table table = new();

            skipWhitespaces();

            if (peek() == '}') {
                move();
                inlineTables.Add(table);
                return table;
            }

            while (!isAtEnd()) {
                List<string> path = parseKey();

                skipWhitespaces();
                expect('=');
                skipWhitespaces();

                Value value = parseValue();
                setValue(table, path, value);

                skipWhitespaces();

                if (peek() == '}') {
                    move();
                    inlineTables.Add(table);
                    return table;
                }

                if (peek() != ',')
                    throw error("Expected ',' or '}' in inline table");

                move();
                skipWhitespaces();

                // TOML 1.0 doesn't allow trailing commas in inline tables.
                if (peek() == '}')
                    throw error("Trailing comma is not allowed in inline table");

                if (peek() == '\n')
                    throw error("Unexpected newline in inline table");
            }

            throw error("Unterminated inline table");
        }

        // ====== Numbers ======

        bool tryParseInteger(string raw, out long result) {
            result = 0;

            string clean = raw.Replace("_", "");

            try {
                if (Regex.IsMatch(raw, "^0x[0-9A-Fa-f](?:_?[0-9A-Fa-f])*$")) 
                    return long.TryParse(clean[2..], NumberStyles.AllowHexSpecifier, CultureInfo.InvariantCulture, out result);
                if (Regex.IsMatch(raw, "^0o[0-7](?:_?[0-7])*$")) {
                    result = Convert.ToInt64(clean[2..], 8);
                    return true;
                }
                if (Regex.IsMatch(raw, "^0b[01](?:_?[01])*$")) {
                    result = Convert.ToInt64(clean[2..], 2);
                    return true;
                }
                if (!Regex.IsMatch(raw, @"^[+-]?(?:0|[1-9](?:_?\d)*)$")) return false;

                return long.TryParse(clean, NumberStyles.AllowLeadingSign, CultureInfo.InvariantCulture, out result);
            } catch {
                throw error($"Integer '{raw}' is out of range");
            }
        }

        bool tryParseFloat(string raw, out double result) {
            result = 0;

            switch (raw) {
                case "inf":
                case "+inf":
                    result = double.PositiveInfinity;
                    return true;

                case "-inf":
                    result = double.NegativeInfinity;
                    return true;

                case "nan":
                case "+nan":
                case "-nan":
                    result = double.NaN;
                    return true;
            }

            // At least a decimal point or exponent is required.
            if (!Regex.IsMatch(raw, @"^[+-]?(?:0|[1-9](?:_?\d)*)(?:(?:\.\d(?:_?\d)*)?(?:[eE][+-]?\d(?:_?\d)*)|\.\d(?:_?\d)*(?:[eE][+-]?\d(?:_?\d)*)?)$"))
                return false;
            string clean = raw.Replace("_", "");
            return double.TryParse(clean, NumberStyles.Float, CultureInfo.InvariantCulture, out result);
        }

        // ====== Date / time ======

        bool tryParseDateTime(string raw, out Value? value) {
            value = null;

            // Offset date-time
            if (Regex.IsMatch(raw, @"^\d{4}-\d{2}-\d{2}[Tt ]\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:[Zz]|[+-]\d{2}:\d{2})$")) {
                string normalized = raw.Replace('t', 'T').Replace('z', 'Z');

                if (!DateTimeOffset.TryParse(normalized, CultureInfo.InvariantCulture, DateTimeStyles.AllowWhiteSpaces, out DateTimeOffset result))
                    throw error($"Invalid offset date-time '{raw}'");

                value = new Value(result);
                return true;
            }

            // Local date-time
            if (Regex.IsMatch(raw, @"^\d{4}-\d{2}-\d{2}[Tt ]\d{2}:\d{2}:\d{2}(?:\.\d+)?$")) {
                if (!DateTime.TryParse(raw.Replace('t', 'T'), CultureInfo.InvariantCulture, DateTimeStyles.AllowWhiteSpaces, out DateTime result))
                    throw error($"Invalid local date-time '{raw}'");

                value = new Value(DateTime.SpecifyKind(result, DateTimeKind.Unspecified));
                return true;
            }

            // Local date
            if (Regex.IsMatch(raw, @"^\d{4}-\d{2}-\d{2}$")) {
                if (!DateOnly.TryParseExact(raw, "yyyy-MM-dd", CultureInfo.InvariantCulture, DateTimeStyles.None, out DateOnly result))
                    throw error($"Invalid local date '{raw}'");

                value = new Value(result);
                return true;
            }

            // Local time
            if (Regex.IsMatch(raw, @"^\d{2}:\d{2}:\d{2}(?:\.\d+)?$")) {
                if (!TimeOnly.TryParse(raw, CultureInfo.InvariantCulture, DateTimeStyles.None, out TimeOnly result))
                    throw error($"Invalid local time '{raw}'");

                value = new Value(result);
                return true;
            }

            return false;
        }

        // ====== Table helpers ======

        Table resolveParentTable(Table start, List<string> path) {
            Table table = start;

            for (int i = 0; i < path.Count - 1; i++) {
                string part = path[i];

                if (inlineTables.Contains(table))
                    throw error("Inline table cannot be extended");

                if (!table.tryGet(part, out Value? existing)) {
                    Table child = new();
                    table.Add(new Property(part, child));
                    table = child;
                    continue;
                }

                if (existing!.isTable()) {
                    table = existing.asTable();
                    continue;
                }

                if (existing.isArray()) {
                    Array array = existing.asArray();

                    if (!arrayTables.Contains(array) ||
                        array.Count == 0 ||
                        !array[^1].isTable())
                        throw error($"'{part}' is not a table");

                    table = array[^1].asTable();
                    continue;
                }

                throw error($"'{part}' is already defined as a value");
            }

            return table;
        }

        void setValue(Table table, List<string> path, Value value) {
            if (path.Count == 0)
                throw error("Key cannot be empty");

            Table parent = resolveParentTable(table, path);
            string name = path[^1];

            if (inlineTables.Contains(parent))
                throw error("Inline table cannot be extended");

            if (parent.tryGet(name, out _))
                throw error($"Key '{name}' is already defined");

            parent.Add(new Property(name, value));
        }
    }

    private sealed class Stringifier {
        private StringBuilder output = new();
        
        // ====== Helpers ======
        void separateSection() {
            if (output.Length == 0) return;
            if (output[^1] != '\n') output.Append('\n');
            if (output.Length < 2 || output[^2] != '\n') output.Append('\n');
        }
        
        // ====== Main function ======
        public string stringify(Table table) {
            output.Clear();
            writeTable(table, [], false);

            // no useless empty line at end of file
            while (output.Length > 0 && output[^1] == '\n') output.Length--;
            if (output.Length > 0) output.Append('\n');

            return output.ToString();
        }

        // ====== Tables ======
        void writeTable(Table table, List<string> path, bool writeHeader) {
            if (writeHeader) {
                separateSection();

                output.Append('[');
                output.Append(stringifyPath(path));
                output.Append("]\n");
            }

            writeValues(table);

            // Normal child tables
            foreach (var property in table) {
                if (!property.value.isTable()) continue;

                List<string> childPath = [..path, property.key];
                writeTable(property.value.asTable(), childPath, true);
            }

            // Arrays of tables
            foreach (var property in table) {
                if (!property.value.isArray()) continue;

                Array array = property.value.asArray();

                if (!isArrayOfTables(array)) continue;

                List<string> childPath = [..path, property.key];
                writeArrayTable(array, childPath);
            }
        }

        void writeValues(Table table) {
            foreach (var property in table) {
                // these are written as [table] later
                if (property.value.isTable()) continue;

                // these are written as [[table]] later
                if (property.value.isArray() && isArrayOfTables(property.value.asArray())) continue;

                output.Append(stringifyKey(property.key));
                output.Append(" = ");
                output.Append(stringifyValue(property.value));
                output.Append('\n');
            }
        }

        void writeArrayTable(Array array, List<string> path) {
            foreach (var value in array) {
                Table table = value.asTable();

                separateSection();

                output.Append("[[");
                output.Append(stringifyPath(path));
                output.Append("]]\n");

                writeValues(table);

                // nested tables belonging to this specific array item
                foreach (var property in table) {
                    if (!property.value.isTable()) continue;

                    List<string> childPath = [..path, property.key];
                    writeTable(property.value.asTable(), childPath, true);
                }

                // nested arrays of tables
                foreach (var property in table) {
                    if (!property.value.isArray()) continue;

                    Array childArray = property.value.asArray();

                    if (!isArrayOfTables(childArray)) continue;

                    List<string> childPath = [..path, property.key];
                    writeArrayTable(childArray, childPath);
                }
            }
        }

        bool isArrayOfTables(Array array) {
            if (array.Count == 0) return false;

            foreach (var value in array)
                if (!value.isTable())
                    return false;

            return true;
        }

        // ====== Values ======
        string stringifyValue(Value value) {
            if (value.isString()) return stringifyString(value.asString());
            if (value.isInt()) return value.asInt().ToString(CultureInfo.InvariantCulture);
            if (value.isDouble()) return stringifyDouble(value.asDouble());
            if (value.isBool()) return value.asBool() ? "true" : "false";
            if (value.isLocalDate()) return value.asLocalDate().ToString("yyyy-MM-dd", CultureInfo.InvariantCulture);
            if (value.isLocalTime()) return stringifyTime(value.asLocalTime());
            if (value.isLocalDateTime()) return stringifyDateTime(value.asLocalDateTime());
            if (value.isOffsetDateTime()) return stringifyOffsetDateTime(value.asOffsetDateTime());
            if (value.isArray()) return stringifyArray(value.asArray());
            // a table appearing inside another value is an inline table.
            if (value.isTable()) return stringifyInlineTable(value.asTable());
            throw new InvalidOperationException($"[Neoluma/TOML] Unsupported TOML value type: {value.raw()?.GetType().Name ?? "null"}");
        }

        string stringifyArray(Array array) {
            StringBuilder result = new();
            result.Append('[');

            for (int i = 0; i < array.Count; i++) {
                if (i != 0) result.Append(", ");
                result.Append(stringifyValue(array[i]));
            }

            result.Append(']');
            return result.ToString();
        }

        string stringifyInlineTable(Table table) {
            StringBuilder result = new();
            result.Append("{ ");

            for (int i = 0; i < table.Count; i++) {
                if (i != 0) result.Append(", ");
                Property property = table[i];

                result.Append(stringifyKey(property.key));
                result.Append(" = ");
                result.Append(stringifyValue(property.value));
            }

            result.Append(" }");
            return result.ToString();
        }

        // ====== Strings ======
        string stringifyString(string value) {
            StringBuilder result = new();
            result.Append('"');

            foreach (char c in value) {
                switch (c) {
                    case '\b': result.Append("\\b"); break;
                    case '\t': result.Append("\\t"); break;
                    case '\n': result.Append("\\n"); break;
                    case '\f': result.Append("\\f"); break;
                    case '\r': result.Append("\\r"); break;
                    case '"': result.Append("\\\""); break;
                    case '\\': result.Append("\\\\"); break;
                    default:
                        // TOML basic strings don't allow raw control chars
                        if (c < 0x20 || c == 0x7F) {
                            result.Append("\\u");
                            result.Append(((int)c).ToString("X4", CultureInfo.InvariantCulture));
                        }
                        else result.Append(c);
                        break;
                }
            }

            result.Append('"');
            return result.ToString();
        }

        // ====== Keys ======
        string stringifyKey(string key) {
            if (key.Length != 0) {
                bool bare = true;

                foreach (char c in key) {
                    if (char.IsAsciiLetterOrDigit(c) || c == '_' || c == '-') continue;

                    bare = false;
                    break;
                }

                if (bare) return key;
            }

            return stringifyString(key);
        }

        string stringifyPath(List<string> path) {
            StringBuilder result = new();

            for (int i = 0; i < path.Count; i++) {
                if (i != 0) result.Append('.');
                result.Append(stringifyKey(path[i]));
            }

            return result.ToString();
        }

        // ====== Numbers ======
        string stringifyDouble(double value) {
            if (double.IsNaN(value)) return "nan";
            if (double.IsPositiveInfinity(value)) return "inf";
            if (double.IsNegativeInfinity(value)) return "-inf";

            string result = value.ToString("R", CultureInfo.InvariantCulture);

            // otherwise 1.0 could become 1 and parse back as integer which would be painful asf
            if (!result.Contains('.') && !result.Contains('e') && !result.Contains('E'))
                result += ".0";

            return result;
        }

        // ====== Dates / times ======
        string stringifyTime(TimeOnly value) {
            string result = value.ToString("HH:mm:ss", CultureInfo.InvariantCulture);
            string fraction = value.ToString("fffffff", CultureInfo.InvariantCulture).TrimEnd('0');
            if (fraction.Length != 0) result += "." + fraction;
            return result;
        }

        string stringifyDateTime(DateTime value) {
            string result = value.ToString("yyyy-MM-dd'T'HH:mm:ss", CultureInfo.InvariantCulture);
            string fraction = value.ToString("fffffff", CultureInfo.InvariantCulture).TrimEnd('0');
            if (fraction.Length != 0) result += "." + fraction;
            return result;
        }

        string stringifyOffsetDateTime(DateTimeOffset value) {
            string result = value.ToString("yyyy-MM-dd'T'HH:mm:ss", CultureInfo.InvariantCulture);
            string fraction = value.ToString("fffffff", CultureInfo.InvariantCulture).TrimEnd('0');
            if (fraction.Length != 0) result += "." + fraction;
            if (value.Offset == TimeSpan.Zero) result += "Z";
            else {
                char sign = value.Offset < TimeSpan.Zero ? '-' : '+';
                TimeSpan offset = value.Offset.Duration();
                
                result += $"{sign}{offset.Hours:00}:{offset.Minutes:00}";
            }

            return result;
        }
    }
}