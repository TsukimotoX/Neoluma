using Neoluma.Libraries;

namespace Neoluma.Tests.TOMLTests;

public class FileTest {
    [Fact]
    public void writeAndParseFile() {
        string directory = Path.Combine(Path.GetTempPath(), $"neoluma-toml-{Guid.NewGuid():N}");
        string path = Path.Combine(directory, "project.nlp");

        Directory.CreateDirectory(directory);

        try {
            TOML.Table root = [
                ("name", "Neoluma"),
                ("version", "0.1.0"),
                ("enabled", true)
            ];

            TOML.writeFile(path, root);

            Assert.True(File.Exists(path));

            TOML.Table parsed = TOML.parseFile(path);
            Assert.Equal("Neoluma", parsed["name"].asString());
            Assert.Equal("0.1.0", parsed["version"].asString());
            Assert.True(parsed["enabled"].asBool());
        }
        finally {
            if (Directory.Exists(directory))
                Directory.Delete(directory, true);
        }
    }
}
