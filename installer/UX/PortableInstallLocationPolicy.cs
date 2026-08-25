using System.IO;

namespace Fabulor.Setup;

internal static class PortableInstallLocationPolicy
{
    public static string GetDefaultInstallFolder()
    {
        var userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        return Path.Combine(userProfile, "Fabulor Portable");
    }

    public static bool IsProtectedLocation(string installFolder)
    {
        if (string.IsNullOrWhiteSpace(installFolder))
        {
            return false;
        }

        string candidate;
        try
        {
            candidate = Path.GetFullPath(installFolder);
        }
        catch (Exception ex) when (ex is ArgumentException or NotSupportedException or PathTooLongException)
        {
            return true;
        }

        return IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles))
            || IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86))
            || IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.Windows));
    }

    private static bool IsSameOrChild(string candidate, string protectedRoot)
    {
        if (string.IsNullOrWhiteSpace(protectedRoot))
        {
            return false;
        }

        var normalizedCandidate = Path.TrimEndingDirectorySeparator(Path.GetFullPath(candidate));
        var normalizedRoot = Path.TrimEndingDirectorySeparator(Path.GetFullPath(protectedRoot));

        return string.Equals(normalizedCandidate, normalizedRoot, StringComparison.OrdinalIgnoreCase)
            || normalizedCandidate.StartsWith(
                normalizedRoot + Path.DirectorySeparatorChar,
                StringComparison.OrdinalIgnoreCase);
    }
}
