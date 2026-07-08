namespace Fabulor.Setup;

public sealed class InstallerFeatureSelection
{
    public bool IncludeDotNetPluginHost { get; set; } = true;

    public bool IncludePythonRuntime { get; set; } = true;

    public bool IncludeTclRuntime { get; set; } = true;

    public bool IncludeThemeAssets { get; set; } = true;

    public bool IncludeGtk4Runtime { get; set; } = true;

    public bool IncludeLegacyGtkCompatibilityData { get; set; } = true;

    public bool IncludeRuntimeDocumentation { get; set; } = true;

    public bool IncludeStartMenuShortcuts { get; set; } = true;

    public bool IncludeShellIntegration { get; set; } = true;

    public bool IncludeTranslations { get; set; } = true;

    public bool IncludeChecksumPlugin { get; set; } = true;

    public bool IncludeExecPlugin { get; set; } = true;

    public bool IncludeFishlimPlugin { get; set; } = true;

    public bool IncludeSysinfoPlugin { get; set; } = true;

    public bool IncludeUpdatePlugin { get; set; } = true;

    public InstallerFeatureSelection Clone()
    {
        return new InstallerFeatureSelection
        {
            IncludeDotNetPluginHost = this.IncludeDotNetPluginHost,
            IncludePythonRuntime = this.IncludePythonRuntime,
            IncludeTclRuntime = this.IncludeTclRuntime,
            IncludeThemeAssets = this.IncludeThemeAssets,
            IncludeGtk4Runtime = this.IncludeGtk4Runtime,
            IncludeLegacyGtkCompatibilityData = this.IncludeLegacyGtkCompatibilityData,
            IncludeRuntimeDocumentation = this.IncludeRuntimeDocumentation,
            IncludeStartMenuShortcuts = this.IncludeStartMenuShortcuts,
            IncludeShellIntegration = this.IncludeShellIntegration,
            IncludeTranslations = this.IncludeTranslations,
            IncludeChecksumPlugin = this.IncludeChecksumPlugin,
            IncludeExecPlugin = this.IncludeExecPlugin,
            IncludeFishlimPlugin = this.IncludeFishlimPlugin,
            IncludeSysinfoPlugin = this.IncludeSysinfoPlugin,
            IncludeUpdatePlugin = this.IncludeUpdatePlugin
        };
    }
}
