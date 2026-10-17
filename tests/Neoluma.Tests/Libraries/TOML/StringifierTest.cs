using Neoluma.Libraries;

namespace Neoluma.Tests.TOMLTests;

public class StringifierTest {
    [Fact]
    public void stringifyPrimitiveValues() {
        TOML.Table root = [
            ("integer", 42),
            ("number", 1.5),
            ("wholeNumber", 1.0),
            ("enabled", true),
            ("disabled", false),
            ("text", "Neoluma")
        ];

        Assert.Equal(
            "integer = 42\n" +
            "number = 1.5\n" +
            "wholeNumber = 1.0\n" +
            "enabled = true\n" +
            "disabled = false\n" +
            "text = \"Neoluma\"\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifySpecialNumbers() {
        TOML.Table root = [
            ("nan", double.NaN),
            ("positive", double.PositiveInfinity),
            ("negative", double.NegativeInfinity)
        ];

        Assert.Equal(
            "nan = nan\n" +
            "positive = inf\n" +
            "negative = -inf\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyStringsAndQuotedKeys() {
        TOML.Table root = [
            ("normal", "hello\nworld"),
            ("quoted key", "value"),
            ("escape", "\"\\\b\f\r\t")
        ];

        Assert.Equal(
            "normal = \"hello\\nworld\"\n" +
            "\"quoted key\" = \"value\"\n" +
            "escape = \"\\\"\\\\\\b\\f\\r\\t\"\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyArraysAndInlineTables() {
        TOML.Table point = [];
        point["x"] = 10;
        point["y"] = 20;

        TOML.Array mixed = ["marker", point];
        TOML.Table root = [
            ("numbers", new TOML.Array { 1, 2, 3 }),
            ("mixed", mixed)
        ];

        Assert.Equal(
            "numbers = [1, 2, 3]\n" +
            "mixed = [\"marker\", { x = 10, y = 20 }]\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyNestedTables() {
        TOML.Table window = [("mode", "windowed")];
        TOML.Table project = [
            ("name", "Neoluma"),
            ("window", window)
        ];
        TOML.Table root = [("project", project)];

        Assert.Equal(
            "[project]\n" +
            "name = \"Neoluma\"\n" +
            "\n" +
            "[project.window]\n" +
            "mode = \"windowed\"\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyArrayOfTables() {
        TOML.Table hammer = [("name", "Hammer")];
        TOML.Table nail = [("name", "Nail")];
        TOML.Array products = [hammer, nail];
        TOML.Table root = [("products", products)];

        Assert.Equal(
            "[[products]]\n" +
            "name = \"Hammer\"\n" +
            "\n" +
            "[[products]]\n" +
            "name = \"Nail\"\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyDateAndTimeValues() {
        TOML.Table root = [];
        root["date"] = new TOML.Value(new DateOnly(2026, 10, 17));
        root["time"] = new TOML.Value(new TimeOnly(19, 23, 45, 120));
        root["local"] = new TOML.Value(new DateTime(2026, 10, 17, 19, 23, 45, 120, DateTimeKind.Unspecified));
        root["offset"] = new TOML.Value(new DateTimeOffset(2026, 10, 17, 19, 23, 45, 120, TimeSpan.FromHours(3)));

        Assert.Equal(
            "date = 2026-10-17\n" +
            "time = 19:23:45.12\n" +
            "local = 2026-10-17T19:23:45.12\n" +
            "offset = 2026-10-17T19:23:45.12+03:00\n",
            TOML.stringify(root)
        );
    }

    [Fact]
    public void stringifyEmptyRootAndKeepSingleFinalNewline() {
        Assert.Equal("", TOML.stringify([]));

        TOML.Table root = [("name", "Neoluma")];
        string text = TOML.stringify(root);

        Assert.Equal("name = \"Neoluma\"\n", text);
        Assert.False(text.EndsWith("\n\n"));
    }

    [Fact]
    public void roundtripProjectData() {
        TOML.Array authors = ["TsukimotoX", "Someone else"];
        TOML.Table project = [
            ("name", "Neoluma"),
            ("version", "0.1.0"),
            ("authors", authors),
            ("enabled", true)
        ];
        TOML.Table root = [("project", project)];

        TOML.Table parsed = TOML.parse(TOML.stringify(root));

        Assert.Equal("Neoluma", parsed["project"]["name"].asString());
        Assert.Equal("0.1.0", parsed["project"]["version"].asString());
        Assert.Equal(2, parsed["project"]["authors"].asArray().Count);
        Assert.True(parsed["project"]["enabled"].asBool());
    }

    [Fact]
    public void roundtripArrayOfTables() {
        TOML.Table hammer = [("name", "Hammer")];
        TOML.Table nail = [("name", "Nail")];
        TOML.Array products = [hammer, nail];
        TOML.Table root = [("products", products)];

        TOML.Table parsed = TOML.parse(TOML.stringify(root));
        TOML.Array parsedProducts = parsed["products"].asArray();

        Assert.Equal(2, parsedProducts.Count);
        Assert.Equal("Hammer", parsedProducts[0]["name"].asString());
        Assert.Equal("Nail", parsedProducts[1]["name"].asString());
    }
}
