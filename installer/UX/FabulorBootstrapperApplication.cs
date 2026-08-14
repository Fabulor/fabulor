using System.Windows;
using System.Windows.Interop;
using System.Windows.Threading;
using System.ComponentModel;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Principal;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Win32;
using WixToolset.BootstrapperApplicationApi;

namespace Fabulor.Setup;

public sealed class FabulorBootstrapperApplication : BootstrapperApplication
{
    private const uint ShellAssociationChanged = 0x08000000;
    private const uint ShellNotifyIdList = 0x0000;
    private const string NoHandoffArgument = "FABULOR_NO_HANDOFF=1";
    private const string FabulorMsiPackageId = "FabulorMsi";
    private const string FabulorMsiUpgradeCode = "{8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11}";
    private const string MainFeatureId = "MainFeature";
    private const string DotNetRuntimeFeatureId = "DotNetRuntimeFeature";
    private const string PythonRuntimeFeatureId = "PythonRuntimeFeature";
    private const string TclRuntimeFeatureId = "TclRuntimeFeature";
    private const string ThemeAssetFeatureId = "ThemeAssetFeature";
    private const string Gtk4RuntimeFeatureId = "Gtk4RuntimeFeature";
    private const string StartMenuFeatureId = "StartMenuFeature";
    private const string DesktopShortcutFeatureId = "DesktopShortcutFeature";
    private const string ShellIntegrationFeatureId = "ShellIntegrationFeature";
    private const string TranslationsFeatureId = "TranslationsFeature";
    private const string ChecksumPluginFeatureId = "ChecksumPluginFeature";
    private const string ExecPluginFeatureId = "ExecPluginFeature";
    private const string FishlimPluginFeatureId = "FishlimPluginFeature";
    private const string SysinfoPluginFeatureId = "SysinfoPluginFeature";

    private int lastResult;
    private bool isFabulorMsiInstalled;
    private bool isCurrentMsiPackageInstalled;
    private bool isDetectedPortableInstall;
    private string? detectedInstalledMsiProductCode;
    private string? detectedInstalledMsiLocation;
    private LaunchAction pendingAction;
    private bool pendingCommandActionRequested;
    private string? detectedInstalledBundleCachePath;
    private readonly Dictionary<string, FeatureState> detectedFeatureStates = new(StringComparer.Ordinal);
    private InstallerFeatureSelection detectedFeatureSelection = new();
    private InstallerFeatureSelection currentPlanFeatureSelection = new();
    private bool currentPlanPortable;
    private MainWindow? window;
    private IntPtr windowHandle;
    private IBootstrapperCommand? command;
    private readonly AutoResetEvent windowReady = new(false);

    [DllImport("shell32.dll")]
    private static extern void SHChangeNotify(uint eventId, uint flags, IntPtr item1, IntPtr item2);

    public FabulorBootstrapperApplication()
    {
        this.Create += this.OnCreate;
        this.Startup += this.OnStartup;
        this.DetectBegin += this.OnDetectBegin;
        this.DetectRelatedBundle += this.OnDetectRelatedBundle;
        this.DetectForwardCompatibleBundle += this.OnDetectForwardCompatibleBundle;
        this.DetectRelatedMsiPackage += this.OnDetectRelatedMsiPackage;
        this.DetectPackageComplete += this.OnDetectPackageComplete;
        this.DetectMsiFeature += this.OnDetectMsiFeature;
        this.DetectComplete += this.OnDetectComplete;
        this.PlanMsiFeature += this.OnPlanMsiFeature;
        this.PlanMsiPackage += this.OnPlanMsiPackage;
        this.PlanPackageBegin += this.OnPlanPackageBegin;
        this.PlanRelatedBundle += this.OnPlanRelatedBundle;
        this.PlanRelatedBundleType += this.OnPlanRelatedBundleType;
        this.PlanRestoreRelatedBundle += this.OnPlanRestoreRelatedBundle;
        this.PlanComplete += this.OnPlanComplete;
        this.ElevateBegin += this.OnElevateBegin;
        this.ElevateComplete += this.OnElevateComplete;
        this.ApplyBegin += this.OnApplyBegin;
        this.ApplyComplete += this.OnApplyComplete;
        this.ExecuteBegin += this.OnExecuteBegin;
        this.ExecuteComplete += this.OnExecuteComplete;
        this.ExecutePackageBegin += this.OnExecutePackageBegin;
        this.ExecutePackageComplete += this.OnExecutePackageComplete;
        this.Progress += this.OnProgress;
        this.ExecuteProgress += this.OnExecuteProgress;
        this.Error += this.OnError;
        this.Shutdown += this.OnShutdown;
    }

    protected override void Run()
    {
        var initialInstallFolder = this.GetInitialInstallFolder();
        var initialPortableMode = this.GetRequestedPortableMode();

        var uiThread = new Thread(() =>
        {
            var application = new System.Windows.Application
            {
                ShutdownMode = ShutdownMode.OnExplicitShutdown
            };

            this.window = new MainWindow(this);
            this.window.SourceInitialized += (_, _) =>
            {
                this.windowHandle = new WindowInteropHelper(this.window).Handle;
                this.window.AppendLog($"Window handle initialised: 0x{this.windowHandle.ToInt64():X}.");
            };
            this.window.Closed += (_, _) => application.Shutdown();
            this.window.InstallFolder = initialInstallFolder;
            this.window.SetPortableMode(initialPortableMode);
            this.window.SetBusy(true);
            this.window.SetDetectedState(false, false);
            this.window.SetStatus("Detecting installed state…");
            this.window.AppendLog("Starting Fabulor custom bootstrapper application.");

            this.windowReady.Set();
            application.Run(this.window);
        });

        uiThread.SetApartmentState(ApartmentState.STA);
        uiThread.Start();

        this.windowReady.WaitOne();
        this.engine.Detect();
        uiThread.Join();
        this.engine.Quit(this.lastResult);
    }

    public void RequestClose()
    {
        this.CloseWindow();
    }

    public void RequestLaunchFabulor()
    {
        if (this.window == null)
        {
            return;
        }

        try
        {
            var installFolder = this.window.InstallFolder;
            if (!System.IO.Path.IsPathFullyQualified(installFolder))
            {
                this.window.ShowError("Fabulor could not be launched because the install folder is invalid.");
                return;
            }

            var executablePath = System.IO.Path.GetFullPath("fabulor.exe", installFolder);
            if (!System.IO.File.Exists(executablePath))
            {
                this.window.ShowError($"Fabulor could not be launched because {executablePath} was not found.");
                return;
            }

            Process.Start(new ProcessStartInfo(executablePath)
            {
                UseShellExecute = true,
                WorkingDirectory = this.window.InstallFolder
            });
            this.CloseWindow();
        }
        catch (Win32Exception ex)
        {
            this.ReportLaunchFailure(ex);
        }
        catch (System.IO.IOException ex)
        {
            this.ReportLaunchFailure(ex);
        }
        catch (UnauthorizedAccessException ex)
        {
            this.ReportLaunchFailure(ex);
        }
        catch (SecurityException ex)
        {
            this.ReportLaunchFailure(ex);
        }
    }

    private void ReportLaunchFailure(Exception exception)
    {
        this.window?.AppendLog($"Launch failure: {exception.GetType().FullName}: {exception.Message}");
        this.window?.ShowError("Fabulor could not be launched. Review the setup details.");
    }

    public void RequestInstall()
    {
        var action = this.isCurrentMsiPackageInstalled ? LaunchAction.Modify : LaunchAction.Install;
        var statusText = this.isCurrentMsiPackageInstalled ? "Planning modify" : "Planning install";
        this.BeginPlan(action, statusText);
    }

    public void RequestRepair()
    {
        this.BeginPlan(LaunchAction.Repair, "Planning repair");
    }

    public void RequestUninstall()
    {
        this.BeginPlan(LaunchAction.Uninstall, "Planning uninstall");
    }

    private void BeginPlan(LaunchAction action, string statusText)
    {
        if (this.window == null)
        {
            return;
        }

        var installFolder = this.window.InstallFolder;
        if (this.pendingCommandActionRequested)
        {
            var requestedInstallFolder = this.GetRequestedInstallFolder();
            if (!string.IsNullOrWhiteSpace(requestedInstallFolder))
            {
                installFolder = requestedInstallFolder;
                this.window.InstallFolder = requestedInstallFolder;
            }
        }

        if (string.IsNullOrWhiteSpace(installFolder))
        {
            this.window.SetStatus("Choose an install folder before starting a bundle action.");
            return;
        }

        this.pendingAction = action;
        var noHandoffRequested = this.HasNoHandoffMarker();

        if (action == LaunchAction.Uninstall
            && !noHandoffRequested
            && this.TryLaunchRegisteredBundleUninstall())
        {
            return;
        }

        if (action == LaunchAction.Uninstall
            && !this.isCurrentMsiPackageInstalled
            && this.TryLaunchRelatedMsiUninstall())
        {
            return;
        }

        if (!noHandoffRequested && this.TryHandOffMaintenanceToInstalledBundle(action))
        {
            return;
        }

        this.CleanupStaleRegistrationsBeforeMaintenancePlan(action);

        if (this.pendingCommandActionRequested && action == LaunchAction.Install)
        {
            this.currentPlanFeatureSelection = new InstallerFeatureSelection();
            this.currentPlanPortable = this.GetRequestedPortableMode();
        }
        else if (this.isFabulorMsiInstalled && (action == LaunchAction.Repair || action == LaunchAction.Uninstall))
        {
            this.currentPlanFeatureSelection = this.detectedFeatureSelection.Clone();
            this.currentPlanPortable = this.isDetectedPortableInstall;
        }
        else
        {
            this.currentPlanFeatureSelection = this.window.FeatureSelection;
            this.currentPlanPortable = this.window.IsPortable;
        }

        this.window.SetBusy(true);
        this.window.SetProgress(0);
        this.window.SetStatus(statusText + "…");
        this.window.AppendLog(this.DescribePlannedAction(action, statusText));
        this.window.AppendLog($"Feature snapshot: dotnet={this.currentPlanFeatureSelection.IncludeDotNetPluginHost}, python={this.currentPlanFeatureSelection.IncludePythonRuntime}, tcl={this.currentPlanFeatureSelection.IncludeTclRuntime}, themeAssets=fixed, gtk4=fixed, startMenu={this.currentPlanFeatureSelection.IncludeStartMenuShortcuts}, desktop={this.currentPlanFeatureSelection.IncludeDesktopShortcut}, shellIntegration={this.currentPlanFeatureSelection.IncludeShellIntegration}, translations={this.currentPlanFeatureSelection.IncludeTranslations}, checksum={this.currentPlanFeatureSelection.IncludeChecksumPlugin}, exec={this.currentPlanFeatureSelection.IncludeExecPlugin}, fishlim={this.currentPlanFeatureSelection.IncludeFishlimPlugin}, sysinfo={this.currentPlanFeatureSelection.IncludeSysinfoPlugin}, portable={this.currentPlanPortable}.");
        if (this.isFabulorMsiInstalled && (action == LaunchAction.Repair || action == LaunchAction.Uninstall))
        {
            this.window.AppendLog("Maintenance action is using the detected installed mode and feature state, not any pending UI edits.");
        }

        this.engine.SetVariableString("InstallFolder", installFolder, true);
        this.engine.SetVariableString("FABULOR_PORTABLE", this.currentPlanPortable ? "1" : string.Empty, true);
        this.engine.Plan(action, BundleScope.PerMachine);
    }

    private string DescribePlannedAction(LaunchAction action, string statusText)
    {
        if (action == LaunchAction.Install && this.isFabulorMsiInstalled)
        {
            return $"{statusText}: Upgrade";
        }

        return $"{statusText}: {action}";
    }

    private string GetInitialInstallFolder()
    {
        var defaultInstallFolder = System.IO.Path.Combine(this.GetDefaultProgramFilesFolder(), "Fabulor");
        var requestedInstallFolder = this.GetRequestedInstallFolder();
        if (!string.IsNullOrWhiteSpace(requestedInstallFolder))
        {
            return requestedInstallFolder;
        }

        return defaultInstallFolder;
    }

    private string GetRequestedInstallFolder()
    {
        var commandLineInstallFolder = this.TryGetInstallFolderFromCommandLine();
        if (!string.IsNullOrWhiteSpace(commandLineInstallFolder))
        {
            return commandLineInstallFolder;
        }

        var installFolder = this.engine.GetVariableString("InstallFolder");
        if (string.IsNullOrWhiteSpace(installFolder))
        {
            return string.Empty;
        }

        var resolvedInstallFolder = this.ResolveInstallFolder(installFolder);
        return !string.IsNullOrWhiteSpace(resolvedInstallFolder) && System.IO.Path.IsPathRooted(resolvedInstallFolder)
            ? resolvedInstallFolder
            : string.Empty;
    }

    private bool GetRequestedPortableMode()
    {
        var commandLinePortableMode = this.TryGetPortableModeFromCommandLine();
        if (commandLinePortableMode.HasValue)
        {
            return commandLinePortableMode.Value;
        }

        var portableValue = this.engine.GetVariableString("FABULOR_PORTABLE");
        return IsTruthyVariable(portableValue);
    }

    private string TryGetInstallFolderFromCommandLine()
    {
        if (string.IsNullOrWhiteSpace(this.command?.CommandLine))
        {
            return string.Empty;
        }

        var match = Regex.Match(
            this.command.CommandLine,
            @"(?:^|\s)InstallFolder=(?:""(?<value>[^""]+)""|(?<value>\S+))",
            RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
        if (!match.Success)
        {
            return string.Empty;
        }

        var installFolder = match.Groups["value"].Value.Trim();
        if (string.IsNullOrWhiteSpace(installFolder))
        {
            return string.Empty;
        }

        var resolvedInstallFolder = this.ResolveInstallFolder(installFolder);
        return !string.IsNullOrWhiteSpace(resolvedInstallFolder) && System.IO.Path.IsPathRooted(resolvedInstallFolder)
            ? resolvedInstallFolder
            : installFolder;
    }

    private bool? TryGetPortableModeFromCommandLine()
    {
        if (string.IsNullOrWhiteSpace(this.command?.CommandLine))
        {
            return null;
        }

        var match = Regex.Match(
            this.command.CommandLine,
            @"(?:^|\s)FABULOR_PORTABLE=(?:""(?<value>[^""]+)""|(?<value>\S+))",
            RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
        if (!match.Success)
        {
            return null;
        }

        return IsTruthyVariable(match.Groups["value"].Value);
    }

    private bool HasNoHandoffMarker()
    {
        if (IsTruthyVariable(this.engine.GetVariableString("FABULOR_NO_HANDOFF")))
        {
            return true;
        }

        if (string.IsNullOrWhiteSpace(this.command?.CommandLine))
        {
            return false;
        }

        return Regex.IsMatch(
            this.command.CommandLine,
            @"(?:^|\s)FABULOR_NO_HANDOFF=(?:""1""|1|true|yes)(?:\s|$)",
            RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
    }

    private static bool IsTruthyVariable(string? value)
    {
        return string.Equals(value, "1", StringComparison.OrdinalIgnoreCase)
            || string.Equals(value, "true", StringComparison.OrdinalIgnoreCase)
            || string.Equals(value, "yes", StringComparison.OrdinalIgnoreCase);
    }

    private string ResolveInstallFolder(string installFolder)
    {
        if (string.IsNullOrWhiteSpace(installFolder))
        {
            return string.Empty;
        }

        if (System.IO.Path.IsPathRooted(installFolder))
        {
            return installFolder;
        }

        var formattedInstallFolder = this.engine.FormatString(installFolder);
        if (!string.IsNullOrWhiteSpace(formattedInstallFolder)
            && !this.ContainsBinderToken(formattedInstallFolder)
            && System.IO.Path.IsPathRooted(formattedInstallFolder))
        {
            return formattedInstallFolder;
        }

        const string programFiles64Token = "[ProgramFiles64Folder]";
        if (installFolder.StartsWith(programFiles64Token, StringComparison.OrdinalIgnoreCase))
        {
            var relativePath = installFolder.Substring(programFiles64Token.Length).TrimStart('\\');
            return string.IsNullOrWhiteSpace(relativePath)
                ? this.GetDefaultProgramFilesFolder()
                : System.IO.Path.Combine(this.GetDefaultProgramFilesFolder(), relativePath);
        }

        const string programFilesToken = "[ProgramFilesFolder]";
        if (installFolder.StartsWith(programFilesToken, StringComparison.OrdinalIgnoreCase))
        {
            var relativePath = installFolder.Substring(programFilesToken.Length).TrimStart('\\');
            return string.IsNullOrWhiteSpace(relativePath)
                ? this.GetDefaultProgramFilesFolder()
                : System.IO.Path.Combine(this.GetDefaultProgramFilesFolder(), relativePath);
        }

        return string.Empty;
    }

    private bool ContainsBinderToken(string value)
    {
        return value.Contains('[') && value.Contains(']');
    }

    private bool IsPortableInstall(string installFolder)
    {
        if (string.IsNullOrWhiteSpace(installFolder))
        {
            return false;
        }

        var portableMarkerPath = System.IO.Path.Combine(installFolder, "portable-mode");
        return System.IO.File.Exists(portableMarkerPath);
    }

    private InstallerFeatureSelection DetectInstalledFeatureSelection(string installFolder, bool isPortable)
    {
        return new InstallerFeatureSelection
        {
            IncludeDotNetPluginHost = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "Runtime", "DotNet", "Fabulor.PluginHost.dll"))
                && System.IO.Directory.Exists(System.IO.Path.Combine(installFolder, "Runtime", "DotNet", "host")),
            IncludePythonRuntime = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "Runtime", "Python314", "python314.dll")),
            IncludeTclRuntime = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "Runtime", "Tcl", "bin", "tcl86t.dll")),
            IncludeStartMenuShortcuts = !isPortable
                && (this.RegistryValueExists(Registry.LocalMachine, @"Software\Fabulor\Installer", "StartMenuShortcuts")
                    || this.RegistryValueExists(Registry.CurrentUser, @"Software\Fabulor\Installer", "StartMenuShortcuts")
                    || this.StartMenuShortcutExists()),
            IncludeDesktopShortcut = !isPortable
                && (this.RegistryValueExists(Registry.LocalMachine, @"Software\Fabulor\Installer", "DesktopShortcut")
                    || this.RegistryValueExists(Registry.CurrentUser, @"Software\Fabulor\Installer", "DesktopShortcut")
                    || this.DesktopShortcutExists()),
            IncludeShellIntegration = !isPortable
                && this.RegistryValueExists(Registry.LocalMachine, @"Software\Fabulor\Installer", "IrcProtocol")
                && this.RegistryValueExists(Registry.LocalMachine, @"Software\Fabulor\Installer", "ThemeAssociation"),
            IncludeTranslations = System.IO.Directory.Exists(System.IO.Path.Combine(installFolder, "share", "locale")),
            IncludeChecksumPlugin = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "plugins", "hcchecksum.dll")),
            IncludeExecPlugin = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "plugins", "hcexec.dll")),
            IncludeFishlimPlugin = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "plugins", "hcfishlim.dll")),
            IncludeSysinfoPlugin = System.IO.File.Exists(System.IO.Path.Combine(installFolder, "plugins", "hcsysinfo.dll"))
        };
    }

    private InstallerFeatureSelection BuildDetectedFeatureSelection(string installFolder, bool isPortable)
    {
        var selection = this.DetectInstalledFeatureSelection(installFolder, isPortable);
        this.ApplyDetectedFeatureState(selection, DotNetRuntimeFeatureId, value => selection.IncludeDotNetPluginHost = value);
        this.ApplyDetectedFeatureState(selection, PythonRuntimeFeatureId, value => selection.IncludePythonRuntime = value);
        this.ApplyDetectedFeatureState(selection, TclRuntimeFeatureId, value => selection.IncludeTclRuntime = value);
        this.ApplyDetectedFeatureState(selection, StartMenuFeatureId, value => selection.IncludeStartMenuShortcuts = value);
        this.ApplyDetectedFeatureState(selection, DesktopShortcutFeatureId, value => selection.IncludeDesktopShortcut = value);
        this.ApplyDetectedFeatureState(selection, ShellIntegrationFeatureId, value => selection.IncludeShellIntegration = value);
        this.ApplyDetectedFeatureState(selection, TranslationsFeatureId, value => selection.IncludeTranslations = value);
        this.ApplyDetectedFeatureState(selection, ChecksumPluginFeatureId, value => selection.IncludeChecksumPlugin = value);
        this.ApplyDetectedFeatureState(selection, ExecPluginFeatureId, value => selection.IncludeExecPlugin = value);
        this.ApplyDetectedFeatureState(selection, FishlimPluginFeatureId, value => selection.IncludeFishlimPlugin = value);
        this.ApplyDetectedFeatureState(selection, SysinfoPluginFeatureId, value => selection.IncludeSysinfoPlugin = value);
        return selection;
    }

    private void ApplyDetectedFeatureState(InstallerFeatureSelection selection, string featureId, Action<bool> assignSelection)
    {
        if (!this.detectedFeatureStates.TryGetValue(featureId, out var state) || state == FeatureState.Unknown)
        {
            return;
        }

        assignSelection(state == FeatureState.Local || state == FeatureState.Source || state == FeatureState.Advertised);
    }

    private bool RegistryValueExists(RegistryKey root, string subkeyPath, string valueName)
    {
        using var subkey = root.OpenSubKey(subkeyPath);
        return subkey?.GetValue(valueName) != null;
    }

    private bool StartMenuShortcutExists()
    {
        return this.StartMenuShortcutExists(Environment.SpecialFolder.CommonPrograms)
            || this.StartMenuShortcutExists(Environment.SpecialFolder.Programs);
    }

    private bool StartMenuShortcutExists(Environment.SpecialFolder folder)
    {
        var programsFolder = Environment.GetFolderPath(folder);
        return !string.IsNullOrWhiteSpace(programsFolder)
            && System.IO.Path.IsPathFullyQualified(programsFolder)
            && System.IO.File.Exists(System.IO.Path.GetFullPath(@"Fabulor\Fabulor.lnk", programsFolder));
    }

    private bool DesktopShortcutExists()
    {
        return this.DesktopShortcutExists(Environment.SpecialFolder.CommonDesktopDirectory)
            || this.DesktopShortcutExists(Environment.SpecialFolder.DesktopDirectory);
    }

    private bool DesktopShortcutExists(Environment.SpecialFolder folder)
    {
        var desktopFolder = Environment.GetFolderPath(folder);
        return !string.IsNullOrWhiteSpace(desktopFolder)
            && System.IO.Path.IsPathFullyQualified(desktopFolder)
            && System.IO.File.Exists(System.IO.Path.GetFullPath("Fabulor.lnk", desktopFolder));
    }

    private void OnPlanMsiFeature(object? sender, PlanMsiFeatureEventArgs e)
    {
        if (!string.Equals(e.PackageId, FabulorMsiPackageId, StringComparison.Ordinal) || this.window == null)
        {
            return;
        }

        if (this.pendingAction != LaunchAction.Install && this.pendingAction != LaunchAction.Modify)
        {
            return;
        }

        var selection = this.currentPlanFeatureSelection;
        var isPortable = this.currentPlanPortable;
        var requestedState = e.FeatureId switch
        {
            MainFeatureId => FeatureState.Local,
            DotNetRuntimeFeatureId => selection.IncludeDotNetPluginHost ? FeatureState.Local : FeatureState.Absent,
            PythonRuntimeFeatureId => selection.IncludePythonRuntime ? FeatureState.Local : FeatureState.Absent,
            TclRuntimeFeatureId => selection.IncludeTclRuntime ? FeatureState.Local : FeatureState.Absent,
            ThemeAssetFeatureId => FeatureState.Local,
            Gtk4RuntimeFeatureId => FeatureState.Local,
            StartMenuFeatureId => !isPortable && selection.IncludeStartMenuShortcuts ? FeatureState.Local : FeatureState.Absent,
            DesktopShortcutFeatureId => !isPortable && selection.IncludeDesktopShortcut ? FeatureState.Local : FeatureState.Absent,
            ShellIntegrationFeatureId => !isPortable && selection.IncludeShellIntegration ? FeatureState.Local : FeatureState.Absent,
            TranslationsFeatureId => selection.IncludeTranslations ? FeatureState.Local : FeatureState.Absent,
            ChecksumPluginFeatureId => selection.IncludeChecksumPlugin ? FeatureState.Local : FeatureState.Absent,
            ExecPluginFeatureId => selection.IncludeExecPlugin ? FeatureState.Local : FeatureState.Absent,
            FishlimPluginFeatureId => selection.IncludeFishlimPlugin ? FeatureState.Local : FeatureState.Absent,
            SysinfoPluginFeatureId => selection.IncludeSysinfoPlugin ? FeatureState.Local : FeatureState.Absent,
            _ => e.RecommendedState
        };

        this.engine.Log(LogLevel.Verbose, $"PlanMsiFeature: feature={e.FeatureId}, recommended={e.RecommendedState}, requested={requestedState}, portable={isPortable}.");
        this.DispatchToWindow(() => this.window?.AppendLog($"PlanMsiFeature: feature={e.FeatureId}, recommended={e.RecommendedState}, requested={requestedState}."));
        e.State = requestedState;
    }

    private void OnPlanPackageBegin(object? sender, PlanPackageBeginEventArgs e)
    {
        if (!string.Equals(e.PackageId, FabulorMsiPackageId, StringComparison.Ordinal))
        {
            return;
        }

        if (this.pendingAction == LaunchAction.Modify)
        {
            e.State = RequestState.ForcePresent;
        }

        this.engine.Log(LogLevel.Verbose, $"PlanPackageBegin: package={e.PackageId}, current={e.CurrentState}, recommended={e.RecommendedState}, requested={e.State}.");
        this.DispatchToWindow(() => this.window?.AppendLog($"PlanPackageBegin: package={e.PackageId}, current={e.CurrentState}, recommended={e.RecommendedState}, requested={e.State}."));
    }

    private void OnPlanMsiPackage(object? sender, PlanMsiPackageEventArgs e)
    {
        if (!string.Equals(e.PackageId, FabulorMsiPackageId, StringComparison.Ordinal))
        {
            return;
        }

        if (this.pendingAction == LaunchAction.Modify)
        {
            typeof(PlanMsiPackageEventArgs)
                .GetProperty(nameof(PlanMsiPackageEventArgs.Action), BindingFlags.Instance | BindingFlags.Public)
                ?.SetValue(e, ActionState.Modify);
            e.ActionMsiProperty = BURN_MSI_PROPERTY.Modify;
        }

        this.engine.Log(LogLevel.Verbose, $"PlanMsiPackage: package={e.PackageId}, action={e.Action}, actionProperty={e.ActionMsiProperty}, uiLevel={e.UiLevel}, shouldExecute={e.ShouldExecute}.");
        this.DispatchToWindow(() => this.window?.AppendLog($"PlanMsiPackage: package={e.PackageId}, action={e.Action}, actionProperty={e.ActionMsiProperty}, shouldExecute={e.ShouldExecute}."));
    }

    private string GetDefaultProgramFilesFolder()
    {
        var programFilesPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
        if (!string.IsNullOrWhiteSpace(programFilesPath))
        {
            return programFilesPath;
        }

        return Environment.GetEnvironmentVariable("ProgramW6432") ?? @"C:\Program Files";
    }

    private void OnApplyComplete(object? sender, ApplyCompleteEventArgs e)
    {
        this.lastResult = e.Status;

        try
        {
            if (e.Status == 0 && this.IsProcessElevated() && this.pendingAction == LaunchAction.Uninstall)
            {
                this.CleanupRegistryArtifactsAfterSuccessfulUninstall();
            }
            else if (e.Status == 0 && this.IsProcessElevated())
            {
                this.CleanupOtherBundleRegistrationsAfterSuccessfulApply();
            }
            else if (e.Status == 0)
            {
                this.engine.Log(LogLevel.Verbose, "Skipping post-apply registry cleanup because the bootstrapper application is not elevated.");
            }
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Post-apply cleanup failed: {ex}");
        }

        if (e.Status == 0)
        {
            NotifyShellAssociationsChanged();
        }

        try
        {
            this.DispatchToWindow(() =>
            {
                if (this.window == null)
                {
                    return;
                }

                this.window.SetBusy(false);
                this.window.SetProgress(100);
                this.window.AppendLog($"ApplyComplete: status=0x{e.Status:X8}, restart={e.Restart}.");
                if (e.Status == 0)
                {
                    this.isFabulorMsiInstalled = this.pendingAction != LaunchAction.Uninstall;
                    this.isCurrentMsiPackageInstalled = this.pendingAction != LaunchAction.Uninstall;
                    this.detectedFeatureSelection = this.currentPlanFeatureSelection.Clone();
                    this.isDetectedPortableInstall = this.currentPlanPortable;
                    this.window.SetStatus("Setup completed successfully.");
                    if (this.pendingAction != LaunchAction.Uninstall)
                    {
                        this.window.ShowCompletion(this.pendingAction == LaunchAction.Repair
                            ? "Fabulor was repaired successfully and is ready to use."
                            : "Fabulor is installed and ready to use.");
                    }
                }
                else
                {
                    this.window.SetDetectedState(this.isFabulorMsiInstalled, this.isCurrentMsiPackageInstalled);
                    this.window.ShowError($"Setup failed with status 0x{e.Status:X8}. Review the details below.");
                }
            });
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"ApplyComplete UI update failed: {ex}");
        }

        if (e.Status == 0 && this.pendingAction == LaunchAction.Uninstall)
        {
            this.CloseWindow();
            return;
        }

        if (this.pendingCommandActionRequested && this.IsNonInteractiveCommandDisplay())
        {
            this.CloseWindow();
            return;
        }

    }

    private void OnApplyBegin(object? sender, ApplyBeginEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetStatus($"Applying {this.pendingAction.ToString().ToLowerInvariant()}…");
            this.window?.AppendLog($"ApplyBegin: phaseCount={e.PhaseCount}.");
        });
    }

    private void OnCreate(object? sender, CreateEventArgs e)
    {
        this.command = e.Command;
    }

    private void OnDetectBegin(object? sender, DetectBeginEventArgs e)
    {
        this.isFabulorMsiInstalled = false;
        this.isCurrentMsiPackageInstalled = false;
        this.isDetectedPortableInstall = false;
        this.detectedInstalledMsiProductCode = null;
        this.detectedInstalledMsiLocation = null;
        this.detectedInstalledBundleCachePath = null;
        this.detectedFeatureSelection = new InstallerFeatureSelection();
        this.detectedFeatureStates.Clear();
    }

    private void OnDetectComplete(object? sender, DetectCompleteEventArgs e)
    {
        try
        {
            this.lastResult = e.Status;

            if (this.isFabulorMsiInstalled
                && (string.IsNullOrWhiteSpace(this.detectedInstalledMsiLocation) || string.IsNullOrWhiteSpace(this.detectedInstalledMsiProductCode)))
            {
                var fallbackInstall = this.TryGetInstalledProductInfoFromRegistry();
                if (fallbackInstall.HasValue)
                {
                    this.detectedInstalledMsiProductCode = fallbackInstall.Value.ProductCode;
                    this.detectedInstalledMsiLocation = fallbackInstall.Value.InstallLocation;
                }
            }

            this.DispatchToWindow(() =>
            {
                if (this.window == null)
                {
                    return;
                }

                if (this.isFabulorMsiInstalled && !string.IsNullOrWhiteSpace(this.detectedInstalledMsiLocation))
                {
                    this.window.InstallFolder = this.detectedInstalledMsiLocation;
                    this.isDetectedPortableInstall = this.IsPortableInstall(this.detectedInstalledMsiLocation);
                    this.detectedFeatureSelection = this.BuildDetectedFeatureSelection(this.detectedInstalledMsiLocation, this.isDetectedPortableInstall);
                    this.window.SetFeatureSelection(this.detectedFeatureSelection.Clone());
                }
                else
                {
                    var requestedInstallFolder = this.GetRequestedInstallFolder();
                    if (!string.IsNullOrWhiteSpace(requestedInstallFolder))
                    {
                        this.window.InstallFolder = requestedInstallFolder;
                    }

                    this.isDetectedPortableInstall = this.GetRequestedPortableMode();

                    this.detectedFeatureSelection = new InstallerFeatureSelection();
                    this.window.SetFeatureSelection(this.detectedFeatureSelection.Clone());
                }

                this.window.SetPortableMode(this.isDetectedPortableInstall);

                this.window.SetBusy(false);
                this.window.SetDetectedState(this.isFabulorMsiInstalled, this.isCurrentMsiPackageInstalled);
                this.window.SetStatus(this.isCurrentMsiPackageInstalled
                    ? this.isDetectedPortableInstall
                        ? "Fabulor is currently installed in portable mode. You can modify, repair, or uninstall it."
                        : "Fabulor is currently installed. You can modify, repair, or uninstall it."
                    : this.isFabulorMsiInstalled
                        ? "An older Fabulor install was detected. You can upgrade it, or uninstall it from this installer."
                        : "Fabulor is not currently installed. Choose a mode and start an install.");
                this.window.AppendLog($"DetectComplete: status=0x{e.Status:X8}, installed={(this.isFabulorMsiInstalled ? "yes" : "no")}.");
            });

            if (e.Status == 0)
            {
                try
                {
                    this.TryStartCommandAction();
                }
                catch (Exception ex)
                {
                    this.engine.Log(LogLevel.Error, $"Command-line action startup failed in OnDetectComplete: {ex}");
                }
            }
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"OnDetectComplete failed: {ex}");
            this.DispatchToWindow(() =>
            {
                this.window?.SetBusy(false);
                this.window?.SetDetectedState(this.isFabulorMsiInstalled, this.isCurrentMsiPackageInstalled);
                this.window?.AppendLog($"DetectComplete failure: {ex.GetType().FullName}: {ex.Message}");
                this.window?.ShowError("Detection failed. Review the setup details.");
            });
        }
    }

    private void OnDetectForwardCompatibleBundle(object? sender, DetectForwardCompatibleBundleEventArgs e)
    {
        this.TrackRelatedBundle(e.BundleCode, e.Version, e.MissingFromCache);

        if (e.MissingFromCache)
        {
            return;
        }

        var bundleCachePath = this.TryGetBundleCachePath(e.BundleCode);
        if (string.IsNullOrWhiteSpace(bundleCachePath))
        {
            return;
        }

        this.detectedInstalledBundleCachePath = bundleCachePath;
        this.engine.Log(LogLevel.Verbose, $"Detected cached related bundle path: {bundleCachePath}");
    }

    private void OnDetectRelatedBundle(object? sender, DetectRelatedBundleEventArgs e)
    {
        this.TrackRelatedBundle(e.ProductCode, e.Version, e.MissingFromCache);
    }

    private void TrackRelatedBundle(string bundleCode, string? version, bool missingFromCache)
    {
        if (missingFromCache || string.IsNullOrWhiteSpace(bundleCode))
        {
            return;
        }
    }

    private void OnDetectPackageComplete(object? sender, DetectPackageCompleteEventArgs e)
    {
        if (!string.Equals(e.PackageId, FabulorMsiPackageId, StringComparison.Ordinal))
        {
            return;
        }

        this.isCurrentMsiPackageInstalled = e.State == PackageState.Present;
        var packageDetectedInstalled = this.isCurrentMsiPackageInstalled;
        this.isFabulorMsiInstalled = this.isFabulorMsiInstalled || packageDetectedInstalled;
        this.engine.Log(LogLevel.Verbose, $"DetectPackageComplete: package={e.PackageId}, state={e.State}, cached={e.Cached}, currentInstalled={this.isCurrentMsiPackageInstalled}, installed={(this.isFabulorMsiInstalled ? "yes" : "no")}.");
    }

    private void OnDetectMsiFeature(object? sender, DetectMsiFeatureEventArgs e)
    {
        if (!string.Equals(e.PackageId, FabulorMsiPackageId, StringComparison.Ordinal))
        {
            return;
        }

        this.detectedFeatureStates[e.FeatureId] = e.State;
    }

    private void OnDetectRelatedMsiPackage(object? sender, DetectRelatedMsiPackageEventArgs e)
    {
        if (!this.GuidEquals(e.UpgradeCode, FabulorMsiUpgradeCode))
        {
            return;
        }

        this.isFabulorMsiInstalled = true;
        this.detectedInstalledMsiProductCode = e.ProductCode;
        this.detectedInstalledMsiLocation = this.TryGetInstalledProductInstallLocationFromRegistry(e.ProductCode);
        this.engine.Log(LogLevel.Verbose, $"Detected related MSI product {e.ProductCode} operation={e.Operation} installLocation='{this.detectedInstalledMsiLocation}'.");
        this.DispatchToWindow(() => this.window?.AppendLog($"DetectRelatedMsiPackage: product={e.ProductCode}, operation={e.Operation}, installLocation='{this.detectedInstalledMsiLocation}'."));
    }

    private void OnError(object? sender, ErrorEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.AppendLog($"Error: type={e.ErrorType}, code=0x{e.ErrorCode:X8}, message={e.ErrorMessage}");
            this.window?.ShowError("Setup reported an error. Review the details below.");
        });
    }

    private void OnExecuteProgress(object? sender, ExecuteProgressEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetProgress(e.OverallPercentage);
        });
    }

    private void OnElevateBegin(object? sender, ElevateBeginEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetStatus("Waiting for elevation approval…");
            this.window?.AppendLog("ElevateBegin: waiting for consent to continue the per-machine apply.");
        });
    }

    private void OnElevateComplete(object? sender, ElevateCompleteEventArgs e)
    {
        this.lastResult = e.Status;

        this.DispatchToWindow(() =>
        {
            if (e.Status == 0)
            {
                this.window?.SetStatus("Elevation approved. Continuing apply…");
                this.window?.AppendLog("ElevateComplete: success.");
                return;
            }

            this.window?.SetBusy(false);
            this.window?.AppendLog($"ElevateComplete: status=0x{e.Status:X8}.");
            this.window?.ShowError($"Elevation failed or was cancelled: 0x{e.Status:X8}.");
        });

        this.RestoreWindowFocusAfterElevation();
    }

    private void RestoreWindowFocusAfterElevation()
    {
        this.DispatchToWindow(() =>
        {
            if (this.window == null)
            {
                return;
            }

            var activated = this.window.RestoreForegroundFocus();
            this.window.AppendLog($"Elevation focus restore: immediate activation={activated}.");
        });

        _ = Task.Run(async () =>
        {
            await Task.Delay(250).ConfigureAwait(false);
            this.DispatchToWindow(() =>
            {
                if (this.window == null)
                {
                    return;
                }

                var activated = this.window.RestoreForegroundFocus();
                this.window.AppendLog($"Elevation focus restore: delayed activation={activated}.");
            });
        });
    }

    private void OnExecuteBegin(object? sender, ExecuteBeginEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetStatus($"Executing {this.pendingAction.ToString().ToLowerInvariant()}…");
            this.window?.AppendLog($"ExecuteBegin: packageCount={e.PackageCount}.");
        });
    }

    private void OnExecuteComplete(object? sender, ExecuteCompleteEventArgs e)
    {
        this.lastResult = e.Status;

        this.DispatchToWindow(() =>
        {
            this.window?.AppendLog($"ExecuteComplete: status=0x{e.Status:X8}.");
        });
    }

    private void OnExecutePackageBegin(object? sender, ExecutePackageBeginEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetStatus($"Executing package {e.PackageId}…");
            this.window?.AppendLog($"ExecutePackageBegin: package={e.PackageId}, action={e.Action}, uiLevel={e.UiLevel}.");
        });
    }

    private void OnExecutePackageComplete(object? sender, ExecutePackageCompleteEventArgs e)
    {
        this.lastResult = e.Status;

        this.DispatchToWindow(() =>
        {
            this.window?.AppendLog($"ExecutePackageComplete: package={e.PackageId}, status=0x{e.Status:X8}, restart={e.Restart}, recommendation={e.Recommendation}.");
        });
    }

    private void OnPlanRelatedBundle(object? sender, PlanRelatedBundleEventArgs e)
    {
    }

    private void OnPlanRelatedBundleType(object? sender, PlanRelatedBundleTypeEventArgs e)
    {
    }

    private void OnPlanRestoreRelatedBundle(object? sender, PlanRestoreRelatedBundleEventArgs e)
    {
    }

    private void OnPlanComplete(object? sender, PlanCompleteEventArgs e)
    {
        this.lastResult = e.Status;

        if (e.Status != 0)
        {
            this.DispatchToWindow(() =>
            {
                this.window?.SetBusy(false);
                this.window?.AppendLog($"PlanComplete: failure status=0x{e.Status:X8}.");
                this.window?.ShowError($"Planning failed with status 0x{e.Status:X8}.");
            });
            return;
        }

        this.DispatchToWindow(() =>
        {
            this.window?.SetStatus($"Applying {this.pendingAction.ToString().ToLowerInvariant()}…");
            this.window?.AppendLog($"PlanComplete: success, applying {this.pendingAction} with parent HWND 0x{this.GetApplyParentWindowHandle().ToInt64():X}.");
        });

        try
        {
            this.engine.Apply(this.GetApplyParentWindowHandle());
        }
        catch (Exception ex)
        {
            this.lastResult = ex.HResult;
            this.DispatchToWindow(() =>
            {
                this.window?.SetBusy(false);
                this.window?.SetStatus($"Apply could not be started: 0x{ex.HResult:X8}.");
                this.window?.AppendLog($"Apply start failed: {ex}");
            });

            throw;
        }
    }

    private void OnProgress(object? sender, ProgressEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.SetProgress(e.OverallPercentage);
        });
    }

    private void OnShutdown(object? sender, ShutdownEventArgs e)
    {
        this.DispatchToWindow(() =>
        {
            this.window?.AppendLog($"Shutdown: action={e.Action}, hr=0x{e.HResult:X8}.");
        });
    }

    private void OnStartup(object? sender, WixToolset.BootstrapperApplicationApi.StartupEventArgs e)
    {
        if (this.command != null)
        {
            this.engine.Log(LogLevel.Standard, $"Startup command: action={this.command.Action}, display={this.command.Display}, relation={this.command.Relation}.");
        }

        this.DispatchToWindow(() =>
        {
            if (this.command == null)
            {
                return;
            }

            this.window?.AppendLog($"Startup: action={this.command.Action}, display={this.command.Display}, relation={this.command.Relation}.");
        });
    }

    private void TryStartCommandAction()
    {
        if (this.pendingCommandActionRequested)
        {
            this.engine.Log(LogLevel.Verbose, "Skipping automatic command-line action because one has already been requested.");
            return;
        }

        if (!this.ShouldAutoRunCommandAction())
        {
            this.engine.Log(LogLevel.Verbose, "Skipping automatic command-line action because the command line did not request non-interactive or explicit-action execution.");
            return;
        }

        var commandLineAction = this.GetCommandLineAction();
        this.engine.Log(LogLevel.Standard, $"Detected command-line action: {commandLineAction}.");
        if (commandLineAction == LaunchAction.Unknown || commandLineAction == LaunchAction.Help || commandLineAction == LaunchAction.Layout || commandLineAction == LaunchAction.Cache)
        {
            this.engine.Log(LogLevel.Verbose, $"No automatic command-line action will be started for {commandLineAction}.");
            return;
        }

        string statusText;
        LaunchAction action;
        switch (commandLineAction)
        {
            case LaunchAction.Install:
            case LaunchAction.UpdateReplace:
            case LaunchAction.UpdateReplaceEmbedded:
                action = LaunchAction.Install;
                statusText = "Planning install";
                break;

            case LaunchAction.Modify:
                action = LaunchAction.Modify;
                statusText = "Planning modify";
                break;

            case LaunchAction.Repair:
                action = LaunchAction.Repair;
                statusText = "Planning repair";
                break;

            case LaunchAction.Uninstall:
            case LaunchAction.UnsafeUninstall:
                action = LaunchAction.Uninstall;
                statusText = "Planning uninstall";
                break;

            default:
                return;
        }

        this.pendingCommandActionRequested = true;
        if (!this.HasNoHandoffMarker() && this.TryHandOffMaintenanceToInstalledBundle(commandLineAction))
        {
            return;
        }

        if (this.HasNoHandoffMarker())
        {
            this.engine.Log(LogLevel.Verbose, "Skipping maintenance handoff because FABULOR_NO_HANDOFF=1 was supplied.");
        }

        this.engine.Log(LogLevel.Standard, $"Scheduling automatic command-line action: {action}.");
        this.DispatchToWindow(() =>
        {
            this.window?.AppendLog($"Starting command-line action automatically: {action} (requested: {commandLineAction}).");
            this.engine.Log(LogLevel.Standard, $"Starting automatic command-line action on the UI thread: {action}.");
            this.BeginPlan(action, statusText);
        });
    }

    private LaunchAction GetCommandLineAction()
    {
        return this.command?.Action ?? LaunchAction.Unknown;
    }

    private bool ShouldAutoRunCommandAction()
    {
        if (this.command == null)
        {
            return false;
        }

        var commandLineAction = this.GetCommandLineAction();
        var nonInteractiveDisplay = this.command.Display == Display.Passive || this.command.Display == Display.None;
        var explicitMaintenanceAction = commandLineAction == LaunchAction.Uninstall
            || commandLineAction == LaunchAction.UnsafeUninstall
            || commandLineAction == LaunchAction.Repair
            || commandLineAction == LaunchAction.Modify;

        this.engine.Log(LogLevel.Verbose, $"Command-action gate: action={commandLineAction}, display={this.command.Display}, nonInteractive={nonInteractiveDisplay}, explicitMaintenance={explicitMaintenanceAction}.");
        return nonInteractiveDisplay || explicitMaintenanceAction;
    }

    private bool TryHandOffMaintenanceToInstalledBundle(LaunchAction action)
    {
        if (this.command == null || !this.IsMaintenanceAction(action))
        {
            return false;
        }

        if (action == LaunchAction.Uninstall || action == LaunchAction.UnsafeUninstall || this.HasNoHandoffMarker())
        {
            return false;
        }

        if (!this.pendingCommandActionRequested && this.isCurrentMsiPackageInstalled)
        {
            return false;
        }

        if (string.IsNullOrWhiteSpace(this.detectedInstalledBundleCachePath))
        {
            this.detectedInstalledBundleCachePath = this.TryGetRegisteredBundleCachePath();
        }

        if (string.IsNullOrWhiteSpace(this.detectedInstalledBundleCachePath) || !System.IO.File.Exists(this.detectedInstalledBundleCachePath))
        {
            return false;
        }

        var sourceBundlePath = this.engine.GetVariableString("WixBundleSourceProcessPath");
        if (string.IsNullOrWhiteSpace(sourceBundlePath) || this.PathsEqual(sourceBundlePath, this.detectedInstalledBundleCachePath))
        {
            return false;
        }

        var relaunchArguments = this.BuildRelaunchArguments(action);
        this.engine.Log(LogLevel.Standard, $"Handing off {action} to installed cached bundle at {this.detectedInstalledBundleCachePath}.");
        this.DispatchToWindow(() =>
        {
            this.window?.SetBusy(true);
            this.window?.SetStatus("Handing off maintenance to the installed bundle…");
            this.window?.AppendLog($"Handing off to cached bundle: {this.detectedInstalledBundleCachePath} {relaunchArguments}");
        });

        if (!this.IsNonInteractiveCommandDisplay())
        {
            var process = this.TryStartInstalledBundle(this.detectedInstalledBundleCachePath, relaunchArguments, out var launchResult);
            if (process == null)
            {
                this.lastResult = launchResult;
                this.DispatchToWindow(() =>
                {
                    this.window?.SetBusy(false);
                    this.window?.SetStatus($"Failed to hand off maintenance to the installed bundle: 0x{launchResult:X8}.");
                });
                return false;
            }

            this.lastResult = 0;
            this.DispatchToWindow(() => this.window?.Close());
            return true;
        }

        _ = Task.Run(() =>
        {
            var result = this.RunInstalledBundle(this.detectedInstalledBundleCachePath, relaunchArguments);
            this.lastResult = result;
            this.DispatchToWindow(() => this.window?.Close());
        });

        return true;
    }

    private bool IsMaintenanceAction(LaunchAction action)
    {
        return action == LaunchAction.Uninstall
            || action == LaunchAction.UnsafeUninstall
            || action == LaunchAction.Repair
            || action == LaunchAction.Modify;
    }

    private bool IsNonInteractiveCommandDisplay()
    {
        return this.command?.Display == Display.Passive || this.command?.Display == Display.None;
    }

    private string BuildRelaunchArguments(LaunchAction action)
    {
        var arguments = action switch
        {
            LaunchAction.Uninstall => "/uninstall",
            LaunchAction.UnsafeUninstall => "/unsafeuninstall",
            LaunchAction.Repair => "/repair",
            LaunchAction.Modify => "/modify",
            _ => string.Empty
        };

        if (this.command?.Display == Display.Passive)
        {
            arguments += " /passive";
        }
        else if (this.command?.Display == Display.None)
        {
            arguments += " /quiet";
        }

        if (!string.IsNullOrWhiteSpace(arguments))
        {
            arguments += " " + NoHandoffArgument;
        }

        return arguments;
    }

    private Process? TryStartInstalledBundle(string bundlePath, string arguments, out int launchResult)
    {
        try
        {
            var process = Process.Start(new ProcessStartInfo(bundlePath, arguments)
            {
                UseShellExecute = true
            });

            if (process == null)
            {
                this.engine.Log(LogLevel.Error, $"Failed to start installed cached bundle at {bundlePath}.");
                launchResult = unchecked((int)0x80004005);
                return null;
            }

            this.engine.Log(LogLevel.Standard, $"Started installed cached bundle at {bundlePath}.");
            launchResult = 0;
            return process;
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to launch installed cached bundle: {ex}");
            launchResult = ex.HResult;
            return null;
        }
    }

    private int RunInstalledBundle(string bundlePath, string arguments)
    {
        using var process = this.TryStartInstalledBundle(bundlePath, arguments, out var launchResult);
        if (process == null)
        {
            return launchResult;
        }

        try
        {
            process.WaitForExit();
            this.engine.Log(LogLevel.Standard, $"Installed cached bundle exited with code 0x{process.ExitCode:X8}.");
            return process.ExitCode;
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed while waiting for installed cached bundle: {ex}");
            return ex.HResult;
        }
    }

    private string? TryGetBundleCachePath(string bundleCode)
    {
        const string uninstallRegistryRoot = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\";

        using var key = Registry.LocalMachine.OpenSubKey(uninstallRegistryRoot + bundleCode);
        return key?.GetValue("BundleCachePath") as string;
    }

    private string? TryGetRegisteredBundleCachePath()
    {
        const string uninstallRegistryRoot = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";

        var bundleName = this.engine.GetVariableString("WixBundleName");
        if (string.IsNullOrWhiteSpace(bundleName))
        {
            return null;
        }

        using var uninstallRoot = Registry.LocalMachine.OpenSubKey(uninstallRegistryRoot);
        if (uninstallRoot == null)
        {
            return null;
        }

        string? newestBundleCachePath = null;
        DateTime newestBundleWriteTime = DateTime.MinValue;

        foreach (var subkeyName in uninstallRoot.GetSubKeyNames())
        {
            using var subkey = uninstallRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("DisplayName") as string, bundleName, StringComparison.Ordinal))
            {
                continue;
            }

            var bundleCachePath = subkey.GetValue("BundleCachePath") as string;
            if (string.IsNullOrWhiteSpace(bundleCachePath) || !System.IO.File.Exists(bundleCachePath))
            {
                continue;
            }

            var writeTime = System.IO.File.GetLastWriteTimeUtc(bundleCachePath);
            if (writeTime > newestBundleWriteTime)
            {
                newestBundleWriteTime = writeTime;
                newestBundleCachePath = bundleCachePath;
            }
        }

        if (!string.IsNullOrWhiteSpace(newestBundleCachePath))
        {
            this.engine.Log(LogLevel.Verbose, $"Resolved newest registered bundle cache path from uninstall registry: {newestBundleCachePath}");
        }

        return newestBundleCachePath;
    }

    private (string ProductCode, string InstallLocation)? TryGetInstalledProductInfoFromRegistry()
    {
        const string uninstallRegistryRoot = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";

        using var uninstallRoot = Registry.LocalMachine.OpenSubKey(uninstallRegistryRoot);
        if (uninstallRoot == null)
        {
            return null;
        }

        foreach (var subkeyName in uninstallRoot.GetSubKeyNames())
        {
            using var subkey = uninstallRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("DisplayName") as string, "Fabulor", StringComparison.Ordinal))
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("WindowsInstaller")?.ToString(), "1", StringComparison.Ordinal))
            {
                continue;
            }

            return (subkeyName, subkey.GetValue("InstallLocation") as string ?? string.Empty);
        }

        return null;
    }

    private string TryGetInstalledProductInstallLocationFromRegistry(string productCode)
    {
        const string uninstallRegistryRoot = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";

        using var uninstallRoot = Registry.LocalMachine.OpenSubKey(uninstallRegistryRoot);
        using var subkey = uninstallRoot?.OpenSubKey(productCode);
        if (subkey == null)
        {
            return string.Empty;
        }

        return subkey.GetValue("InstallLocation") as string ?? string.Empty;
    }

    private bool TryLaunchRelatedMsiUninstall()
    {
        if (this.command?.Display == Display.Embedded)
        {
            this.engine.Log(LogLevel.Verbose, "Skipping related MSI uninstall because this bundle is running embedded as a related bundle.");
            return false;
        }

        if (string.IsNullOrWhiteSpace(this.detectedInstalledMsiProductCode))
        {
            return false;
        }

        var arguments = $"/x{this.detectedInstalledMsiProductCode}";
        if (this.command?.Display == Display.Passive)
        {
            arguments += " /passive";
        }
        else if (this.command?.Display == Display.None)
        {
            arguments += " /quiet";
        }

        try
        {
            var process = Process.Start(new ProcessStartInfo("msiexec.exe", arguments)
            {
                UseShellExecute = true
            });

            if (process == null)
            {
                return false;
            }

            this.engine.Log(LogLevel.Standard, $"Launched related MSI uninstall for product {this.detectedInstalledMsiProductCode}.");
            this.DispatchToWindow(() =>
            {
                this.window?.SetBusy(true);
                this.window?.SetStatus("Uninstalling the detected installed Fabulor MSI…");
                this.window?.AppendLog($"Launching msiexec {arguments}");
            });

            _ = Task.Run(() =>
            {
                var result = this.WaitForRelatedMsiUninstall(process);
                this.lastResult = result;
                this.DispatchToWindow(() =>
                {
                    if (result == 0)
                    {
                        this.CleanupRegistryArtifactsAfterSuccessfulUninstall();
                        this.window?.AppendLog("Related MSI uninstall completed successfully.");
                        this.window?.SetStatus("Fabulor was uninstalled successfully.");
                        this.window?.Close();
                    }
                    else
                    {
                        this.window?.SetBusy(false);
                        this.window?.AppendLog($"Related MSI uninstall failed with status 0x{result:X8}.");
                        this.window?.ShowError($"Detected MSI uninstall failed with status 0x{result:X8}.");
                    }
                });
            });

            return true;
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to launch related MSI uninstall: {ex}");
            this.DispatchToWindow(() =>
            {
                this.window?.SetStatus("Failed to launch uninstall for the detected installed MSI.");
                this.window?.AppendLog($"Related MSI uninstall failed: {ex.Message}");
            });
            return false;
        }
    }

    private int WaitForRelatedMsiUninstall(Process process)
    {
        using (process)
        {
            try
            {
                process.WaitForExit();
                this.engine.Log(LogLevel.Standard, $"Related MSI uninstall exited with code 0x{process.ExitCode:X8}.");
                return process.ExitCode;
            }
            catch (Exception ex)
            {
                this.engine.Log(LogLevel.Error, $"Failed while waiting for related MSI uninstall: {ex}");
                return ex.HResult;
            }
        }
    }

    private bool TryLaunchRegisteredBundleUninstall()
    {
        if (this.command?.Display == Display.Embedded)
        {
            this.engine.Log(LogLevel.Verbose, "Skipping registered bundle uninstall relaunch because this bundle is running embedded as a related bundle.");
            return false;
        }

        if (string.IsNullOrWhiteSpace(this.detectedInstalledBundleCachePath))
        {
            this.detectedInstalledBundleCachePath = this.TryGetRegisteredBundleCachePath();
        }

        if (string.IsNullOrWhiteSpace(this.detectedInstalledBundleCachePath)
            || !System.IO.File.Exists(this.detectedInstalledBundleCachePath))
        {
            return false;
        }

        var sourceBundlePath = this.engine.GetVariableString("WixBundleSourceProcessPath");
        if (!string.IsNullOrWhiteSpace(sourceBundlePath)
            && this.PathsEqual(sourceBundlePath, this.detectedInstalledBundleCachePath))
        {
            this.engine.Log(LogLevel.Verbose, $"Skipping registered bundle uninstall relaunch because the current source bundle already matches {this.detectedInstalledBundleCachePath}.");
            return false;
        }

        try
        {
            var arguments = this.BuildRelaunchArguments(LaunchAction.Uninstall);
            var process = Process.Start(new ProcessStartInfo(this.detectedInstalledBundleCachePath, arguments)
            {
                UseShellExecute = true
            });

            if (process == null)
            {
                return false;
            }

            this.engine.Log(LogLevel.Standard, $"Launched registered bundle uninstall from {this.detectedInstalledBundleCachePath}.");
            this.DispatchToWindow(() =>
            {
                this.window?.SetBusy(true);
                this.window?.SetStatus("Launching uninstall from the registered Fabulor Setup bundle…");
                this.window?.AppendLog($"Launching cached bundle uninstall: {this.detectedInstalledBundleCachePath} {arguments}");
                this.window?.Close();
            });
            this.lastResult = 0;
            return true;
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to launch registered bundle uninstall: {ex}");
            this.DispatchToWindow(() =>
            {
                this.window?.SetStatus("Failed to launch uninstall from the registered bundle.");
                this.window?.AppendLog($"Registered bundle uninstall failed: {ex.Message}");
            });
            return false;
        }
    }

    private void CleanupRegistryArtifactsAfterSuccessfulUninstall()
    {
        if (this.TryGetInstalledProductInfoFromRegistry().HasValue)
        {
            this.engine.Log(LogLevel.Verbose, "Skipping post-uninstall registry cleanup because an installed Fabulor MSI is still registered.");
            return;
        }

        var currentBundlePath = this.engine.GetVariableString("WixBundleSourceProcessPath");
        var bundleName = this.engine.GetVariableString("WixBundleName");

        this.RemoveOtherBundleUninstallRegistrations(bundleName, currentBundlePath, preserveNewestRegisteredBundle: false);
        this.RemoveDependencyRegistrations();
        this.RemoveShellIntegrationRegistryRoots();
        this.RemoveInstallerRegistryRoots();
    }

    private void CleanupOtherBundleRegistrationsAfterSuccessfulApply()
    {
        var currentBundlePath = this.engine.GetVariableString("WixBundleSourceProcessPath");
        var bundleName = this.engine.GetVariableString("WixBundleName");

        this.RemoveOtherBundleUninstallRegistrations(bundleName, currentBundlePath, preserveNewestRegisteredBundle: true);
    }

    private void CleanupStaleRegistrationsBeforeMaintenancePlan(LaunchAction action)
    {
        if (action != LaunchAction.Modify &&
            action != LaunchAction.Repair &&
            action != LaunchAction.Uninstall)
        {
            return;
        }

        try
        {
            var currentBundlePath = this.engine.GetVariableString("WixBundleSourceProcessPath");
            var bundleName = this.engine.GetVariableString("WixBundleName");
            var registeredBundlePath = this.TryGetRegisteredBundleCachePath();
            var preservedBundlePath = !string.IsNullOrWhiteSpace(registeredBundlePath) ? registeredBundlePath : currentBundlePath;
            var preservedBundleCode = this.TryGetBundleCodeFromCachePath(preservedBundlePath);

            this.engine.Log(LogLevel.Verbose, $"Pre-plan stale registration cleanup: action={action}, preservedBundlePath='{preservedBundlePath}', preservedBundleCode='{preservedBundleCode}'.");
            this.RemoveOtherBundleUninstallRegistrations(bundleName, preservedBundlePath, preserveNewestRegisteredBundle: true);
            this.RemoveStaleBundleDependencyDependents(preservedBundleCode);
            this.RemoveStaleMsiDependencyRegistrations();
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Pre-plan stale registration cleanup failed: {ex}");
            this.DispatchToWindow(() => this.window?.AppendLog($"Pre-plan cleanup failed: {ex.Message}"));
        }
    }

    private void RemoveOtherBundleUninstallRegistrations(string? bundleName, string? currentBundlePath, bool preserveNewestRegisteredBundle)
    {
        if (string.IsNullOrWhiteSpace(bundleName))
        {
            return;
        }

        const string uninstallRegistryRoot = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";
        using var uninstallRoot = Registry.LocalMachine.OpenSubKey(uninstallRegistryRoot, writable: true);
        if (uninstallRoot == null)
        {
            return;
        }

        if (preserveNewestRegisteredBundle)
        {
            currentBundlePath = this.ResolveCurrentRegisteredBundleCachePath(uninstallRoot, bundleName, currentBundlePath);
        }

        foreach (var subkeyName in uninstallRoot.GetSubKeyNames())
        {
            using var subkey = uninstallRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("DisplayName") as string, bundleName, StringComparison.Ordinal))
            {
                continue;
            }

            var bundleCachePath = subkey.GetValue("BundleCachePath") as string;
            if (!string.IsNullOrWhiteSpace(currentBundlePath)
                && !string.IsNullOrWhiteSpace(bundleCachePath)
                && this.PathsEqual(currentBundlePath, bundleCachePath))
            {
                continue;
            }

            try
            {
                uninstallRoot.DeleteSubKeyTree(subkeyName, throwOnMissingSubKey: false);
                this.engine.Log(LogLevel.Standard, $"Removed leftover bundle uninstall registration '{subkeyName}'.");
            }
            catch (Exception ex)
            {
                this.engine.Log(LogLevel.Error, $"Failed to remove leftover bundle uninstall registration '{subkeyName}': {ex}");
            }
        }
    }

    private void RemoveStaleBundleDependencyDependents(string? preservedBundleCode)
    {
        const string dependentsPath = @"SOFTWARE\Classes\Installer\Dependencies\Fabulor.Setup.Bundle\Dependents";
        using var dependentsKey = Registry.LocalMachine.OpenSubKey(dependentsPath, writable: true);
        if (dependentsKey == null)
        {
            return;
        }

        foreach (var dependentCode in dependentsKey.GetSubKeyNames())
        {
            if (!string.IsNullOrWhiteSpace(preservedBundleCode) &&
                string.Equals(dependentCode, preservedBundleCode, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            try
            {
                dependentsKey.DeleteSubKeyTree(dependentCode, throwOnMissingSubKey: false);
                this.engine.Log(LogLevel.Standard, $"Removed stale Fabulor Setup dependency dependent '{dependentCode}'.");
            }
            catch (Exception ex)
            {
                this.engine.Log(LogLevel.Error, $"Failed to remove stale Fabulor Setup dependency dependent '{dependentCode}': {ex}");
            }
        }
    }

    private void RemoveStaleMsiDependencyRegistrations()
    {
        const string dependencyRegistryRoot = @"SOFTWARE\Classes\Installer\Dependencies";
        using var dependencyRoot = Registry.LocalMachine.OpenSubKey(dependencyRegistryRoot, writable: true);
        if (dependencyRoot == null)
        {
            return;
        }

        foreach (var subkeyName in dependencyRoot.GetSubKeyNames())
        {
            using var subkey = dependencyRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("DisplayName") as string, "Fabulor", StringComparison.Ordinal))
            {
                continue;
            }

            if (!string.IsNullOrWhiteSpace(this.detectedInstalledMsiProductCode) &&
                subkeyName.StartsWith(this.detectedInstalledMsiProductCode, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            try
            {
                dependencyRoot.DeleteSubKeyTree(subkeyName, throwOnMissingSubKey: false);
                this.engine.Log(LogLevel.Standard, $"Removed stale Fabulor MSI dependency registration '{subkeyName}'.");
            }
            catch (Exception ex)
            {
                this.engine.Log(LogLevel.Error, $"Failed to remove stale Fabulor MSI dependency registration '{subkeyName}': {ex}");
            }
        }
    }

    private string? ResolveCurrentRegisteredBundleCachePath(RegistryKey uninstallRoot, string bundleName, string? currentBundlePath)
    {
        string? newestBundleCachePath = null;
        DateTime newestBundleWriteTime = DateTime.MinValue;

        foreach (var subkeyName in uninstallRoot.GetSubKeyNames())
        {
            using var subkey = uninstallRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            if (!string.Equals(subkey.GetValue("DisplayName") as string, bundleName, StringComparison.Ordinal))
            {
                continue;
            }

            var bundleCachePath = subkey.GetValue("BundleCachePath") as string;
            if (string.IsNullOrWhiteSpace(bundleCachePath) || !System.IO.File.Exists(bundleCachePath))
            {
                continue;
            }

            if (!string.IsNullOrWhiteSpace(currentBundlePath) && this.PathsEqual(currentBundlePath, bundleCachePath))
            {
                return bundleCachePath;
            }

            var writeTime = System.IO.File.GetLastWriteTimeUtc(bundleCachePath);
            if (writeTime > newestBundleWriteTime)
            {
                newestBundleWriteTime = writeTime;
                newestBundleCachePath = bundleCachePath;
            }
        }

        if (!string.IsNullOrWhiteSpace(newestBundleCachePath))
        {
            this.engine.Log(LogLevel.Verbose, $"Preserving newest registered bundle cache path: {newestBundleCachePath}");
        }

        return newestBundleCachePath;
    }

    private string? TryGetBundleCodeFromCachePath(string? bundleCachePath)
    {
        if (string.IsNullOrWhiteSpace(bundleCachePath))
        {
            return null;
        }

        try
        {
            var directoryName = System.IO.Path.GetFileName(System.IO.Path.GetDirectoryName(bundleCachePath));
            return Regex.IsMatch(directoryName ?? string.Empty, @"^\{[0-9A-Fa-f-]{36}\}$")
                ? directoryName
                : null;
        }
        catch
        {
            return null;
        }
    }

    private void RemoveDependencyRegistrations()
    {
        const string dependencyRegistryRoot = @"SOFTWARE\Classes\Installer\Dependencies";
        using var dependencyRoot = Registry.LocalMachine.OpenSubKey(dependencyRegistryRoot, writable: true);
        if (dependencyRoot == null)
        {
            return;
        }

        foreach (var subkeyName in dependencyRoot.GetSubKeyNames())
        {
            using var subkey = dependencyRoot.OpenSubKey(subkeyName);
            if (subkey == null)
            {
                continue;
            }

            var displayName = subkey.GetValue("DisplayName") as string;
            var shouldDelete = string.Equals(subkeyName, "Fabulor.Setup.Bundle", StringComparison.Ordinal)
                || string.Equals(displayName, "Fabulor", StringComparison.Ordinal)
                || string.Equals(displayName, "Fabulor Setup", StringComparison.Ordinal);
            if (!shouldDelete)
            {
                continue;
            }

            try
            {
                dependencyRoot.DeleteSubKeyTree(subkeyName, throwOnMissingSubKey: false);
                this.engine.Log(LogLevel.Standard, $"Removed leftover dependency registration '{subkeyName}'.");
            }
            catch (Exception ex)
            {
                this.engine.Log(LogLevel.Error, $"Failed to remove leftover dependency registration '{subkeyName}': {ex}");
            }
        }
    }

    private void RemoveInstallerRegistryRoots()
    {
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Fabulor\Installer");
        this.DeleteRegistryTreeIfExists(Registry.CurrentUser, @"Software\Fabulor\Installer");
        this.DeleteRegistryTreeIfEmpty(Registry.LocalMachine, @"Software\Fabulor");
        this.DeleteRegistryTreeIfEmpty(Registry.CurrentUser, @"Software\Fabulor");
    }

    private static void NotifyShellAssociationsChanged()
    {
        SHChangeNotify(ShellAssociationChanged, ShellNotifyIdList, IntPtr.Zero, IntPtr.Zero);
    }

    private void RemoveShellIntegrationRegistryRoots()
    {
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\Fabulor.Theme");
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\Fabulor.Url.Irc");
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\Fabulor.Url.IrcSecure");
        // Remove the retired ZoiteChat extension when cleaning older installs.
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\.zct");
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\.hct");
        this.DeleteRegistryValueIfEquals(
            Registry.LocalMachine,
            @"Software\RegisteredApplications",
            "Fabulor",
            @"Software\Fabulor\Capabilities");
        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Fabulor\Capabilities");
        this.DeleteOwnedLegacyIrcProtocolRegistration();
        this.DeleteRegistryTreeIfEmpty(Registry.LocalMachine, @"Software\Fabulor");
    }

    private void DeleteOwnedLegacyIrcProtocolRegistration()
    {
        const string commandPath = @"Software\Classes\irc\shell\open\command";
        using var commandKey = Registry.LocalMachine.OpenSubKey(commandPath);
        var command = commandKey?.GetValue(null) as string;
        if (string.IsNullOrWhiteSpace(command)
            || !command.Contains("fabulor.exe", StringComparison.OrdinalIgnoreCase))
        {
            return;
        }

        this.DeleteRegistryTreeIfExists(Registry.LocalMachine, @"Software\Classes\irc");
    }

    private void DeleteRegistryValueIfEquals(
        RegistryKey root,
        string subkeyPath,
        string valueName,
        string expectedValue)
    {
        try
        {
            using var subkey = root.OpenSubKey(subkeyPath, writable: true);
            var currentValue = subkey?.GetValue(valueName) as string;
            if (!string.Equals(currentValue, expectedValue, StringComparison.OrdinalIgnoreCase))
            {
                return;
            }

            subkey!.DeleteValue(valueName, throwOnMissingValue: false);
            this.engine.Log(LogLevel.Verbose, $"Deleted owned registry value '{root.Name}\\{subkeyPath}\\{valueName}'.");
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to delete registry value '{root.Name}\\{subkeyPath}\\{valueName}': {ex}");
        }
    }

    private void DeleteRegistryTreeIfExists(RegistryKey root, string subkeyPath)
    {
        try
        {
            root.DeleteSubKeyTree(subkeyPath, throwOnMissingSubKey: false);
            this.engine.Log(LogLevel.Verbose, $"Deleted registry tree '{root.Name}\\{subkeyPath}' if it existed.");
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to delete registry tree '{root.Name}\\{subkeyPath}': {ex}");
        }
    }

    private void DeleteRegistryTreeIfEmpty(RegistryKey root, string subkeyPath)
    {
        using var subkey = root.OpenSubKey(subkeyPath);
        if (subkey == null)
        {
            return;
        }

        if (subkey.SubKeyCount != 0 || subkey.ValueCount != 0)
        {
            return;
        }

        try
        {
            root.DeleteSubKeyTree(subkeyPath, throwOnMissingSubKey: false);
            this.engine.Log(LogLevel.Verbose, $"Deleted empty registry tree '{root.Name}\\{subkeyPath}'.");
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to delete empty registry tree '{root.Name}\\{subkeyPath}': {ex}");
        }
    }

    private bool GuidEquals(string? left, string? right)
    {
        if (Guid.TryParse(left, out var leftGuid) && Guid.TryParse(right, out var rightGuid))
        {
            return leftGuid == rightGuid;
        }

        return string.Equals(left, right, StringComparison.OrdinalIgnoreCase);
    }

    private bool PathsEqual(string left, string right)
    {
        try
        {
            return string.Equals(
                System.IO.Path.GetFullPath(left),
                System.IO.Path.GetFullPath(right),
                StringComparison.OrdinalIgnoreCase);
        }
        catch
        {
            return string.Equals(left, right, StringComparison.OrdinalIgnoreCase);
        }
    }

    private string GetCurrentBundleVersion()
    {
        try
        {
            return this.engine.GetVariableString("WixBundleVersion") ?? string.Empty;
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to read WixBundleVersion: {ex}");
            return string.Empty;
        }
    }

    private bool VersionsEqual(string? left, string? right)
    {
        if (string.IsNullOrWhiteSpace(left) || string.IsNullOrWhiteSpace(right))
        {
            return false;
        }

        if (Version.TryParse(left, out var leftVersion) && Version.TryParse(right, out var rightVersion))
        {
            return this.NormalizeVersion(leftVersion).Equals(this.NormalizeVersion(rightVersion));
        }

        return string.Equals(left, right, StringComparison.OrdinalIgnoreCase);
    }

    private bool IsProcessElevated()
    {
        try
        {
            using var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to determine process elevation: {ex}");
            return false;
        }
    }

    private Version NormalizeVersion(Version version)
    {
        return new Version(
            version.Major,
            version.Minor,
            Math.Max(version.Build, 0),
            Math.Max(version.Revision, 0));
    }

    private void CloseWindow()
    {
        try
        {
            var activeWindow = this.window;
            if (activeWindow == null || activeWindow.Dispatcher.HasShutdownStarted || activeWindow.Dispatcher.HasShutdownFinished)
            {
                return;
            }

            if (activeWindow.Dispatcher.CheckAccess())
            {
                activeWindow.Close();
                return;
            }

            activeWindow.Dispatcher.Invoke(() =>
            {
                if (!activeWindow.Dispatcher.HasShutdownStarted && !activeWindow.Dispatcher.HasShutdownFinished)
                {
                    activeWindow.Close();
                }
            }, DispatcherPriority.Send);
        }
        catch (Exception ex)
        {
            this.engine.Log(LogLevel.Error, $"Failed to close bootstrapper window: {ex}");
        }
    }

    private void DispatchToWindow(Action action)
    {
        if (this.window == null)
        {
            return;
        }

        if (this.window.Dispatcher.CheckAccess())
        {
            action();
            return;
        }

        this.window.Dispatcher.BeginInvoke(action, DispatcherPriority.Normal);
    }

    private IntPtr GetApplyParentWindowHandle()
    {
        if (this.window == null)
        {
            return IntPtr.Zero;
        }

        if (this.windowHandle != IntPtr.Zero)
        {
            return this.windowHandle;
        }

        if (this.window.Dispatcher.CheckAccess())
        {
            return new WindowInteropHelper(this.window).Handle;
        }

        return this.window.Dispatcher.Invoke(() => new WindowInteropHelper(this.window).Handle);
    }
}
