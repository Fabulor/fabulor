using System.IO;
using System.Runtime.InteropServices;
using System.Text;

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
            candidate = CanonicalizeExistingPathPrefix(Path.GetFullPath(installFolder));
        }
        catch (Exception ex) when (ex is ArgumentException
            or NotSupportedException
            or PathTooLongException
            or IOException
            or UnauthorizedAccessException)
        {
            return true;
        }

        return ContainsReparsePoint(candidate)
            || IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles))
            || IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86))
            || IsSameOrChild(candidate, Environment.GetFolderPath(Environment.SpecialFolder.Windows));
    }

    private static string CanonicalizeExistingPathPrefix(string candidate)
    {
        var missingSegments = new Stack<string>();
        var existingPath = candidate;

        while (!Directory.Exists(existingPath))
        {
            var segment = Path.GetFileName(existingPath);
            var parent = Path.GetDirectoryName(existingPath);
            if (string.IsNullOrWhiteSpace(parent) || string.IsNullOrWhiteSpace(segment))
            {
                break;
            }

            missingSegments.Push(segment);
            existingPath = parent;
        }

        var canonicalPath = ExpandLongPath(existingPath);
        while (missingSegments.Count > 0)
        {
            canonicalPath = Path.Combine(canonicalPath, missingSegments.Pop());
        }

        return Path.GetFullPath(canonicalPath);
    }

    private static string ExpandLongPath(string path)
    {
        var requiredLength = NativeMethods.GetLongPathName(path, null, 0);
        if (requiredLength == 0)
        {
            return path;
        }

        var buffer = new StringBuilder((int)requiredLength);
        var resultLength = NativeMethods.GetLongPathName(path, buffer, (uint)buffer.Capacity);
        return resultLength == 0 ? path : buffer.ToString();
    }

    private static bool ContainsReparsePoint(string candidate)
    {
        var existingPath = candidate;
        while (!Directory.Exists(existingPath))
        {
            var parent = Path.GetDirectoryName(existingPath);
            if (string.IsNullOrWhiteSpace(parent) || string.Equals(parent, existingPath, StringComparison.OrdinalIgnoreCase))
            {
                return false;
            }

            existingPath = parent;
        }

        for (var current = new DirectoryInfo(existingPath); current is not null; current = current.Parent)
        {
            if ((current.Attributes & FileAttributes.ReparsePoint) != 0)
            {
                return true;
            }
        }

        return false;
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

    private static class NativeMethods
    {
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern uint GetLongPathName(string shortPath, StringBuilder? longPath, uint bufferLength);
    }
}
