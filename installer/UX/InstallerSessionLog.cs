using System.Globalization;
using System.IO;
using System.Text;

namespace Fabulor.Setup;

internal sealed class InstallerSessionLog : IDisposable
{
    private const int SuccessfulLogRetentionCount = 10;
    private static readonly TimeSpan FailedLogRetention = TimeSpan.FromDays(90);
    private readonly object syncRoot = new();
    private readonly StreamWriter writer;
    private bool failed;
    private bool disposed;

    public InstallerSessionLog()
    {
        this.DirectoryPath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "Fabulor",
            "Installer",
            "Logs");
        Directory.CreateDirectory(this.DirectoryPath);
        PruneOldLogs(this.DirectoryPath);

        var timestamp = DateTime.Now.ToString("yyyy-MM-dd-HHmmss", CultureInfo.InvariantCulture);
        this.FilePath = Path.Combine(this.DirectoryPath, $"FabulorSetup-{timestamp}-{Environment.ProcessId}.log");
        this.writer = new StreamWriter(this.FilePath, append: false, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false))
        {
            AutoFlush = true
        };
    }

    public string DirectoryPath { get; }

    public string FilePath { get; private set; }

    public void Write(string message)
    {
        lock (this.syncRoot)
        {
            if (this.disposed)
            {
                return;
            }

            this.writer.WriteLine($"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff zzz}] {message}");
        }
    }

    public void MarkFailure()
    {
        this.failed = true;
    }

    public void Dispose()
    {
        lock (this.syncRoot)
        {
            if (this.disposed)
            {
                return;
            }

            this.disposed = true;
            this.writer.Dispose();

            if (this.failed && File.Exists(this.FilePath))
            {
                var failedPath = Path.ChangeExtension(this.FilePath, ".failed.log");
                File.Move(this.FilePath, failedPath, overwrite: true);
                this.FilePath = failedPath;
            }
        }
    }

    private static void PruneOldLogs(string directoryPath)
    {
        try
        {
            var logs = new DirectoryInfo(directoryPath)
                .EnumerateFiles("FabulorSetup-*.log", SearchOption.TopDirectoryOnly)
                .OrderByDescending(file => file.LastWriteTimeUtc)
                .ToArray();

            foreach (var file in logs.Where(file => file.Name.EndsWith(".failed.log", StringComparison.OrdinalIgnoreCase)
                                                    && DateTime.UtcNow - file.LastWriteTimeUtc > FailedLogRetention))
            {
                file.Delete();
            }

            foreach (var file in logs.Where(file => !file.Name.EndsWith(".failed.log", StringComparison.OrdinalIgnoreCase))
                                     .Skip(SuccessfulLogRetentionCount))
            {
                file.Delete();
            }
        }
        catch (IOException)
        {
        }
        catch (UnauthorizedAccessException)
        {
        }
    }
}
