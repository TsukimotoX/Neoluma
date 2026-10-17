using Neoluma.Core.Extras;
using Neoluma.Libraries;

namespace Neoluma.CLI;

public static class CLIParser {
    // stuff required for parseProjectFile
    static string getString(TOML.Table table, string key, string defaultValue) {
        if (table.tryGet(key, out TOML.Value? value) && value!.isString())
            return value.asString();

        return defaultValue;
    }

    static List<string> getStringArray(TOML.Table table, string key) {
        List<string> result = [];

        if (!table.tryGet(key, out TOML.Value? value) || !value!.isArray())
            return result;

        foreach (TOML.Value item in value.asArray())
            if (item.isString())
                result.Add(item.asString());

        return result;
    }

    static Dictionary<string, string> extractMap(TOML.Table root, string key) {
        Dictionary<string, string> result = [];

        if (!root.tryGet(key, out TOML.Value? value) || !value!.isTable())
            return result;

        foreach (TOML.Property property in value.asTable())
            if (property.value.isString())
                result[property.key] = property.value.asString();

        return result;
    }
    
    public static ProjectConfig parseProjectFile(string file) {
        TOML.Table root = TOML.parseFile(file);
        ProjectConfig config = new();

        if (root.tryGet("project", out TOML.Value? projectValue) && projectValue!.isTable()) {
            TOML.Table project = projectValue.asTable();
            config.name = getString(project, "name", config.name);
            config.version = getString(project, "version", config.version);
            config.author = getStringArray(project, "authors");
            config.license = getString(project, "license", config.license);
            config.output = getString(project, "output", config.output);
            config.sourceFolder = getString(project, "sourceFolder", config.sourceFolder);
            config.buildFolder = getString(project, "buildFolder", config.buildFolder);
        }

        config.tasks = extractMap(root, "tasks");
        config.dependencies = extractMap(root, "dependencies");
        config.tests = extractMap(root, "tests");
        config.languagePacks = extractMap(root, "languagePacks");

        config.sourcePath = Path.GetDirectoryName(Path.GetFullPath(file))!;
        return config;
    }
}