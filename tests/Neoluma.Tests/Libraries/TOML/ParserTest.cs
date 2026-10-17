using Neoluma.Libraries;

namespace Neoluma.Tests.TOMLTests;

public class ParserTest {
    [Fact]
    public void parsePrimitiveValues() {
        TOML.Table root = TOML.parse("""
        text = "hello"
        integer = 67
        number = 6.9
        enabled = true
        disabled = false
        """);

        Assert.Equal("hello", root["text"].asString());
        Assert.Equal(67L, root["integer"].asInt());
        Assert.Equal(6.9, root["number"].asDouble());
        Assert.True(root["enabled"].asBool());
        Assert.False(root["disabled"].asBool());
    }

    [Fact]
    public void parseIntegerAndFloatFormats() {
        TOML.Table root = TOML.parse("""
        decimal = 1_000_000
        hex = 0xDEAD_BEEF
        octal = 0o755
        binary = 0b1010_0110
        exponent = 5e+22
        fraction = -0.01
        positiveInfinity = +inf
        negativeInfinity = -inf
        notNumber = nan
        """);

        Assert.Equal(1_000_000L, root["decimal"].asInt());
        Assert.Equal(0xDEADBEEFL, root["hex"].asInt());
        Assert.Equal(493L, root["octal"].asInt());
        Assert.Equal(166L, root["binary"].asInt());
        Assert.Equal(5e22, root["exponent"].asDouble());
        Assert.Equal(-0.01, root["fraction"].asDouble());
        Assert.True(double.IsPositiveInfinity(root["positiveInfinity"].asDouble()));
        Assert.True(double.IsNegativeInfinity(root["negativeInfinity"].asDouble()));
        Assert.True(double.IsNaN(root["notNumber"].asDouble()));
    }

    [Fact]
    public void parseStrings() {
        TOML.Table root = TOML.parse("""
        basic = "hello\nworld"
        literal = 'C:\Users\test'
        unicode = "\u0416\U0001F44B"
        """);

        Assert.Equal("hello\nworld", root["basic"].asString());
        Assert.Equal(@"C:\Users\test", root["literal"].asString());
        Assert.Equal("Ж👋", root["unicode"].asString());
    }

    [Fact]
    public void parseMultilineStrings() {
        string source = """"
        basic = """
        hello
        world
        """
        literal = '''
        hello
        world
        '''
        continued = """
        hello \
            world
        """
        """";

        TOML.Table root = TOML.parse(source);

        Assert.Equal("hello\nworld\n", root["basic"].asString());
        Assert.Equal("hello\nworld\n", root["literal"].asString());
        Assert.Equal("hello world\n", root["continued"].asString());
    }

    [Fact]
    public void parseArraysAndInlineTables() {
        TOML.Table root = TOML.parse("""
        numbers = [1, 2, 3,]
        mixed = [1, "two", true]
        point = { x = 10, y = 20 }
        multiline = [
            1,
            # comment
            2,
            3,
        ]
        """);

        TOML.Array numbers = root["numbers"].asArray();
        Assert.Equal(3, numbers.Count);
        Assert.Equal(1L, numbers[0].asInt());
        Assert.Equal(3L, numbers[2].asInt());

        TOML.Array mixed = root["mixed"].asArray();
        Assert.Equal("two", mixed[1].asString());
        Assert.True(mixed[2].asBool());

        Assert.Equal(10L, root["point"]["x"].asInt());
        Assert.Equal(20L, root["point"]["y"].asInt());
        Assert.Equal(3, root["multiline"].asArray().Count);
    }

    [Fact]
    public void parseKeysAndTables() {
        TOML.Table root = TOML.parse("""
        bare-key = "bare"
        "quoted key" = "quoted"
        physical.color = "orange"

        [project]
        name = "Neoluma"

        [project.window]
        width = 800
        """);

        Assert.Equal("bare", root["bare-key"].asString());
        Assert.Equal("quoted", root["quoted key"].asString());
        Assert.Equal("orange", root["physical"]["color"].asString());
        Assert.Equal("Neoluma", root["project"]["name"].asString());
        Assert.Equal(800L, root["project"]["window"]["width"].asInt());
    }

    [Fact]
    public void parseArrayOfTables() {
        TOML.Table root = TOML.parse("""
        [[products]]
        name = "Hammer"
        sku = 738594937

        [[products]]
        name = "Nail"
        sku = 284758393
        """);

        TOML.Array products = root["products"].asArray();

        Assert.Equal(2, products.Count);
        Assert.Equal("Hammer", products[0]["name"].asString());
        Assert.Equal(738594937L, products[0]["sku"].asInt());
        Assert.Equal("Nail", products[1]["name"].asString());
    }

    [Fact]
    public void parseNestedArrayOfTables() {
        TOML.Table root = TOML.parse("""
        [[fruits]]
        name = "apple"

        [[fruits.varieties]]
        name = "red delicious"

        [[fruits.varieties]]
        name = "granny smith"

        [[fruits]]
        name = "banana"

        [[fruits.varieties]]
        name = "plantain"
        """);

        TOML.Array fruits = root["fruits"].asArray();

        Assert.Equal(2, fruits.Count);
        Assert.Equal("apple", fruits[0]["name"].asString());
        Assert.Equal(2, fruits[0]["varieties"].asArray().Count);
        Assert.Equal("granny smith", fruits[0]["varieties"].asArray()[1]["name"].asString());
        Assert.Equal("banana", fruits[1]["name"].asString());
        Assert.Equal("plantain", fruits[1]["varieties"].asArray()[0]["name"].asString());
    }

    [Fact]
    public void parseDateAndTimeValues() {
        TOML.Table root = TOML.parse("""
        offset = 1979-05-27T07:32:00Z
        localDateTime = 1979-05-27T07:32:00
        localDate = 1979-05-27
        localTime = 07:32:00.123
        """);

        Assert.True(root["offset"].isOffsetDateTime());
        Assert.True(root["localDateTime"].isLocalDateTime());
        Assert.True(root["localDate"].isLocalDate());
        Assert.True(root["localTime"].isLocalTime());

        Assert.Equal(new DateOnly(1979, 5, 27), root["localDate"].asLocalDate());
        Assert.Equal(new TimeOnly(7, 32, 0, 123), root["localTime"].asLocalTime());
    }

    [Fact]
    public void parseCommentsBomAndCrLf() {
        TOML.Table root = TOML.parse("\uFEFF# comment\r\nname = \"Neoluma\" # inline\r\n\r\n[project]\r\nversion = \"0.1.0\"\r\n");

        Assert.Equal("Neoluma", root["name"].asString());
        Assert.Equal("0.1.0", root["project"]["version"].asString());
    }

    [Fact]
    public void rejectDuplicateAndConflictingDefinitions() {
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = 1\nvalue = 2"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("[project]\n[project]"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("project = 1\n[project]"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("project = { name = \"x\" }\n[project]"));
    }

    [Fact]
    public void rejectMalformedToml() {
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = nope"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = \"unterminated"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = \"\\q\""));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = [1, 2"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("value = { a = 1, }"));
        Assert.Throws<TOML.ParseError>(() => TOML.parse("date = 2026-13-01"));
    }

    [Fact]
    public void parseErrorContainsLocation() {
        TOML.ParseError error = Assert.Throws<TOML.ParseError>(() => TOML.parse("name = \"ok\"\nvalue = nope"));

        Assert.Equal(2, error.line);
        Assert.True(error.column > 0);
    }
}
