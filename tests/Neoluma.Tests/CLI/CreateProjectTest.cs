using Neoluma.Core.Extras;
using Neoluma.Libraries;

namespace Neoluma.Tests.CLITests;

[CollectionDefinition("CurrentDirectory", DisableParallelization = true)]
public class CurrentDirectoryCollection { }

[Collection("CurrentDirectory")]
public class CreateProjectTest {
    [Fact]
    public void createProjectWritesExpectedFiles() {
        string originalDirectory = Directory.GetCurrentDirectory();
        string temporaryDirectory = Path.Combine(Path.GetTempPath(), $"neoluma-project-{Guid.NewGuid():N}");
        Directory.CreateDirectory(temporaryDirectory);

        try {
            Directory.SetCurrentDirectory(temporaryDirectory);

            ProjectConfig config = new() {
                name = "Test Project",
                version = "0.1.0",
                author = ["TsukimotoX", "Someone else"],
                license = "mit",
                output = "exe",
                sourceFolder = "src/",
                buildFolder = "build/"
            };

            Neoluma.CLI.CLI.createProject(config);

            string projectDirectory = Path.Combine(temporaryDirectory, config.name);
            string projectFile = Path.Combine(projectDirectory, "test_project.nlp");
            string mainFile = Path.Combine(projectDirectory, "src", "main.nm");
            string licenseFile = Path.Combine(projectDirectory, "LICENSE");

            Assert.True(File.Exists(projectFile));
            Assert.True(File.Exists(mainFile));
            Assert.True(File.Exists(licenseFile));

            Assert.Contains("@entry", File.ReadAllText(mainFile));
            Assert.False(string.IsNullOrWhiteSpace(File.ReadAllText(licenseFile)));

            TOML.Table root = TOML.parseFile(projectFile);
            TOML.Value project = root["project"];

            Assert.Equal(config.name, project["name"].asString());
            Assert.Equal(config.version, project["version"].asString());
            Assert.Equal(config.license, project["license"].asString());
            Assert.Equal(config.output, project["output"].asString());
            Assert.Equal(config.sourceFolder, project["sourceFolder"].asString());
            Assert.Equal(config.buildFolder, project["buildFolder"].asString());

            TOML.Array authors = project["authors"].asArray();
            Assert.Equal(2, authors.Count);
            Assert.Equal("TsukimotoX", authors[0].asString());
            Assert.Equal("Someone else", authors[1].asString());

            Assert.Equal("neoluma run --debug", root["tasks"]["dev"].asString());
        }
        finally {
            Directory.SetCurrentDirectory(originalDirectory);

            if (Directory.Exists(temporaryDirectory))
                Directory.Delete(temporaryDirectory, true);
        }
    }
}
