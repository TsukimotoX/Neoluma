using Neoluma.Libraries;

namespace Neoluma.Tests.TOMLTests;

public class ValueTest {
    [Fact]
    public void validateConstructors() {
        Assert.Null(new TOML.Value().raw());
        Assert.IsType<long>(new TOML.Value(1).raw());
        Assert.IsType<double>(new TOML.Value(1.5).raw());
        Assert.IsType<string>(new TOML.Value("test").raw());
        Assert.IsType<bool>(new TOML.Value(false).raw());
        Assert.IsType<TOML.Array>(new TOML.Value(new TOML.Array { 1, 2 }).raw());
        Assert.IsType<TOML.Table>(new TOML.Value(new TOML.Table()).raw());
        Assert.IsType<DateOnly>(new TOML.Value(new DateOnly(2026, 10, 17)).raw());
        Assert.IsType<TimeOnly>(new TOML.Value(new TimeOnly(12, 30, 15)).raw());
        Assert.IsType<DateTime>(new TOML.Value(new DateTime(2026, 10, 17, 12, 30, 15)).raw());
        Assert.IsType<DateTimeOffset>(new TOML.Value(new DateTimeOffset(2026, 10, 17, 12, 30, 15, TimeSpan.Zero)).raw());
    }

    [Fact]
    public void validateTypeChecks() {
        Assert.True(new TOML.Value(1).isInt());
        Assert.True(new TOML.Value(1.5).isDouble());
        Assert.True(new TOML.Value("test").isString());
        Assert.True(new TOML.Value(false).isBool());
        Assert.True(new TOML.Value(new TOML.Array()).isArray());
        Assert.True(new TOML.Value(new TOML.Table()).isTable());
        Assert.True(new TOML.Value(new DateOnly(2026, 10, 17)).isLocalDate());
        Assert.True(new TOML.Value(new TimeOnly(12, 30, 15)).isLocalTime());
        Assert.True(new TOML.Value(new DateTime(2026, 10, 17, 12, 30, 15)).isLocalDateTime());
        Assert.True(new TOML.Value(new DateTimeOffset(2026, 10, 17, 12, 30, 15, TimeSpan.Zero)).isOffsetDateTime());
    }

    [Fact]
    public void validateGetters() {
        Assert.Equal(5L, new TOML.Value(5).asInt());
        Assert.Equal(5.5, new TOML.Value(5.5).asDouble());
        Assert.Equal(5.0, new TOML.Value(5).asDouble());
        Assert.Equal("test", new TOML.Value("test").asString());
        Assert.True(new TOML.Value(true).asBool());

        TOML.Array array = [1, 2, 3];
        TOML.Table table = [("name", "Neoluma")];

        Assert.Same(array, new TOML.Value(array).asArray());
        Assert.Same(table, new TOML.Value(table).asTable());
    }

    [Fact]
    public void validateTableIndexing() {
        TOML.Table table = [
            ("name", "Neoluma"),
            ("version", "0.1.0")
        ];

        Assert.Equal("Neoluma", table["name"].asString());
        Assert.True(table.tryGet("version", out TOML.Value? version));
        Assert.Equal("0.1.0", version!.asString());
        Assert.False(table.tryGet("missing", out _));
        Assert.Throws<KeyNotFoundException>(() => table["missing"]);

        table["version"] = "0.2.0";
        table["enabled"] = true;

        Assert.Equal("0.2.0", table["version"].asString());
        Assert.True(table["enabled"].asBool());
    }

    [Fact]
    public void validateValueIndexingAndImplicitConversions() {
        TOML.Table table = [];
        TOML.Value value = table;

        value["name"] = "Neoluma";
        value["version"] = 1;

        Assert.Equal("Neoluma", value["name"].asString());
        Assert.Equal(1L, value["version"].asInt());

        TOML.Value integer = 1;
        TOML.Value number = 1.5;
        TOML.Value text = "test";
        TOML.Value boolean = true;
        TOML.Array array = [1, "two", false];
        TOML.Value arrayValue = array;

        Assert.Equal(1L, integer.asInt());
        Assert.Equal(1.5, number.asDouble());
        Assert.Equal("test", text.asString());
        Assert.True(boolean.asBool());
        Assert.Same(array, arrayValue.asArray());
        Assert.Throws<InvalidOperationException>(() => integer["nope"]);
    }
}
