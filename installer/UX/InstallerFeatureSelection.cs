namespace Fabulor.Setup;

public sealed class InstallerFeatureSelection
{
    public bool IncludeStartMenuShortcuts { get; set; } = true;

    public bool IncludeShellIntegration { get; set; } = true;

    public bool IncludeTranslations { get; set; } = true;

    public bool IncludeChecksumPlugin { get; set; } = true;

    public bool IncludeExecPlugin { get; set; } = true;

    public bool IncludeFishlimPlugin { get; set; } = true;

    public bool IncludeSysinfoPlugin { get; set; } = true;

    public bool IncludeUpdatePlugin { get; set; } = true;
}
