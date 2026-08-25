using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Interop;
using Forms = global::System.Windows.Forms;

namespace Fabulor.Setup;

public partial class MainWindow : Window
{
    private readonly FabulorBootstrapperApplication bootstrapper;
    private readonly InstallerSessionLog sessionLog = new();
    private bool rememberedStartMenuShortcuts = true;
    private bool rememberedDesktopShortcut = true;
    private bool rememberedShellIntegration = true;
    private bool showingInstallFolderWarning;
    private bool changingInstallFolderProgrammatically;
    private bool changingPortableModeProgrammatically;
    private bool installFolderEditedByUser;
    private readonly bool trackingInstallFolderEdits;
    private string installedDefaultFolder = string.Empty;
    private string portableDefaultFolder = string.Empty;
    private string lastErrorDetails = string.Empty;

    public MainWindow(FabulorBootstrapperApplication bootstrapper)
    {
        this.bootstrapper = bootstrapper;
        this.InitializeComponent();
        this.trackingInstallFolderEdits = true;
        this.UpdateModeSummary();
        this.Closed += (_, _) => this.sessionLog.Dispose();
    }

    public string InstallFolder
    {
        get => this.InstallFolderTextBox.Text.Trim();
        set
        {
            this.changingInstallFolderProgrammatically = true;
            try
            {
                this.InstallFolderTextBox.Text = value;
            }
            finally
            {
                this.changingInstallFolderProgrammatically = false;
            }
        }
    }

    public bool IsPortable => this.PortableModeCheckBox.IsChecked == true;

    public InstallerFeatureSelection FeatureSelection => new()
    {
        IncludeDotNetPluginHost = this.DotNetPluginHostCheckBox.IsChecked == true,
        IncludePythonRuntime = this.PythonRuntimeCheckBox.IsChecked == true,
        IncludeTclRuntime = this.TclRuntimeCheckBox.IsChecked == true,
        IncludeStartMenuShortcuts = this.StartMenuShortcutsCheckBox.IsChecked == true,
        IncludeDesktopShortcut = this.DesktopShortcutCheckBox.IsChecked is true,
        IncludeShellIntegration = this.ShellIntegrationCheckBox.IsChecked == true,
        IncludeTranslations = this.TranslationsCheckBox.IsChecked == true,
        IncludeChecksumPlugin = this.ChecksumPluginCheckBox.IsChecked == true,
        IncludeExecPlugin = this.ExecPluginCheckBox.IsChecked == true,
        IncludeFishlimPlugin = this.FishlimPluginCheckBox.IsChecked == true,
        IncludeSysinfoPlugin = this.SysinfoPluginCheckBox.IsChecked == true
    };

    public void SetPortableMode(bool isPortable)
    {
        this.changingPortableModeProgrammatically = true;
        try
        {
            this.PortableModeCheckBox.IsChecked = isPortable;
            this.RefreshOptionState();
        }
        finally
        {
            this.changingPortableModeProgrammatically = false;
        }
    }

    public void SetInstallFolderDefaults(string installedFolder, string portableFolder)
    {
        this.installedDefaultFolder = installedFolder;
        this.portableDefaultFolder = portableFolder;
    }

    public void SetFeatureSelection(InstallerFeatureSelection selection)
    {
        this.rememberedStartMenuShortcuts = selection.IncludeStartMenuShortcuts;
        this.rememberedDesktopShortcut = selection.IncludeDesktopShortcut;
        this.rememberedShellIntegration = selection.IncludeShellIntegration;

        this.DotNetPluginHostCheckBox.IsChecked = selection.IncludeDotNetPluginHost;
        this.PythonRuntimeCheckBox.IsChecked = selection.IncludePythonRuntime;
        this.TclRuntimeCheckBox.IsChecked = selection.IncludeTclRuntime;
        this.StartMenuShortcutsCheckBox.IsChecked = selection.IncludeStartMenuShortcuts;
        this.DesktopShortcutCheckBox.IsChecked = selection.IncludeDesktopShortcut;
        this.ShellIntegrationCheckBox.IsChecked = selection.IncludeShellIntegration;
        this.TranslationsCheckBox.IsChecked = selection.IncludeTranslations;
        this.ChecksumPluginCheckBox.IsChecked = selection.IncludeChecksumPlugin;
        this.ExecPluginCheckBox.IsChecked = selection.IncludeExecPlugin;
        this.FishlimPluginCheckBox.IsChecked = selection.IncludeFishlimPlugin;
        this.SysinfoPluginCheckBox.IsChecked = selection.IncludeSysinfoPlugin;
        this.RefreshOptionState();
    }

    public void AppendLog(string message)
    {
        if (string.IsNullOrWhiteSpace(message))
        {
            return;
        }

        this.sessionLog.Write(message);
        var builder = new StringBuilder(this.LogTextBox.Text);
        if (builder.Length > 0)
        {
            builder.AppendLine();
        }

        builder.Append(message);
        this.LogTextBox.Text = builder.ToString();
        this.LogTextBox.ScrollToEnd();
    }

    public void ShowError(string message)
    {
        this.SetStatus(message);
        this.lastErrorDetails = this.LogTextBox.Text;
        this.CopyErrorDetailsButton.IsEnabled = !string.IsNullOrWhiteSpace(this.lastErrorDetails);
        this.DetailsExpander.IsExpanded = true;
        this.sessionLog.MarkFailure();
    }

    public void ShowCompletion(string message)
    {
        this.SetBusy(false);
        this.StatePanel.Visibility = Visibility.Collapsed;
        this.ConfigurationPanel.Visibility = Visibility.Collapsed;
        this.CompletionTextBlock.Text = message;
        this.CompletionPanel.Visibility = Visibility.Visible;
        this.InstallButton.Visibility = Visibility.Collapsed;
        this.RepairButton.Visibility = Visibility.Collapsed;
        this.UninstallButton.Visibility = Visibility.Collapsed;
        this.LaunchButton.Visibility = Visibility.Visible;
        this.LaunchButton.Focus();
    }

    public void SetBusy(bool isBusy)
    {
        this.InstallButton.IsEnabled = !isBusy;
        this.RepairButton.IsEnabled = !isBusy;
        this.UninstallButton.IsEnabled = !isBusy;
        this.PortableModeCheckBox.IsEnabled = !isBusy;
        this.InstallFolderTextBox.IsEnabled = !isBusy;
        this.BrowseInstallFolderButton.IsEnabled = !isBusy;
        this.DotNetPluginHostCheckBox.IsEnabled = !isBusy;
        this.PythonRuntimeCheckBox.IsEnabled = !isBusy;
        this.TclRuntimeCheckBox.IsEnabled = !isBusy;
        this.TranslationsCheckBox.IsEnabled = !isBusy;
        this.ChecksumPluginCheckBox.IsEnabled = !isBusy;
        this.ExecPluginCheckBox.IsEnabled = !isBusy;
        this.FishlimPluginCheckBox.IsEnabled = !isBusy;
        this.SysinfoPluginCheckBox.IsEnabled = !isBusy;

        if (isBusy)
        {
            this.StartMenuShortcutsCheckBox.IsEnabled = false;
            this.DesktopShortcutCheckBox.IsEnabled = false;
            this.ShellIntegrationCheckBox.IsEnabled = false;
            return;
        }

        this.RefreshOptionState();
    }

    public void SetDetectedState(bool isInstalled, bool isCurrentPackageInstalled)
    {
        this.StateHeadingTextBlock.Text = isInstalled ? "Maintain Fabulor" : "Ready to install";
        this.StateDescriptionTextBlock.Text = isCurrentPackageInstalled
            ? "Fabulor is installed. You can change installed features, repair the installation, or uninstall it."
            : isInstalled
                ? "An older Fabulor installation was detected. Review the options below, then upgrade it."
                : "Fabulor is not currently installed. Review the location below, then select Install.";
        this.ConfigurationHeadingTextBlock.Text = isInstalled ? "Installed features" : "Installation";
        this.InstallButton.Content = isInstalled
            ? isCurrentPackageInstalled ? "_Modify" : "_Upgrade"
            : "_Install";
        this.RepairButton.Visibility = isCurrentPackageInstalled ? Visibility.Visible : Visibility.Collapsed;
        this.UninstallButton.Visibility = isInstalled ? Visibility.Visible : Visibility.Collapsed;
        this.InstallButton.Visibility = Visibility.Visible;
        this.LaunchButton.Visibility = Visibility.Collapsed;
        this.InstallButton.Focus();
    }

    public void SetProgress(int percentage)
    {
        this.ProgressBar.Value = Math.Clamp(percentage, 0, 100);
    }

    public void SetStatus(string message)
    {
        this.StatusTextBlock.Text = message;
    }

    public bool RestoreForegroundFocus()
    {
        if (!this.IsVisible)
        {
            this.Show();
        }

        if (this.WindowState == WindowState.Minimized)
        {
            this.WindowState = WindowState.Normal;
        }

        var handle = new WindowInteropHelper(this).Handle;
        var activated = this.Activate();
        if (handle != IntPtr.Zero)
        {
            NativeMethods.BringWindowToTop(handle);
            activated = NativeMethods.SetForegroundWindow(handle) || activated;
        }

        if (!activated)
        {
            var wasTopmost = this.Topmost;
            this.Topmost = true;
            this.Topmost = wasTopmost;
            activated = this.Activate();
        }

        this.Focus();
        return activated;
    }

    private void UpdateModeSummary()
    {
        this.ModeSummaryTextBlock.Text = this.IsPortable
            ? "Portable mode keeps configuration beside the executable and does not create Windows shortcuts or registrations."
            : "Installed mode stores user configuration in the Fabulor profile and enables the selected Windows integrations.";
    }

    private void RefreshOptionState()
    {
        this.UpdateModeSummary();
        if (this.IsPortable)
        {
            this.rememberedStartMenuShortcuts = this.StartMenuShortcutsCheckBox.IsChecked == true;
            this.rememberedDesktopShortcut = this.DesktopShortcutCheckBox.IsChecked is true;
            this.rememberedShellIntegration = this.ShellIntegrationCheckBox.IsChecked == true;
            this.StartMenuShortcutsCheckBox.IsChecked = false;
            this.DesktopShortcutCheckBox.IsChecked = false;
            this.ShellIntegrationCheckBox.IsChecked = false;
            this.StartMenuShortcutsCheckBox.IsEnabled = false;
            this.DesktopShortcutCheckBox.IsEnabled = false;
            this.ShellIntegrationCheckBox.IsEnabled = false;
            return;
        }

        this.StartMenuShortcutsCheckBox.IsEnabled = true;
        this.DesktopShortcutCheckBox.IsEnabled = true;
        this.ShellIntegrationCheckBox.IsEnabled = true;
        this.StartMenuShortcutsCheckBox.IsChecked = this.rememberedStartMenuShortcuts;
        this.DesktopShortcutCheckBox.IsChecked = this.rememberedDesktopShortcut;
        this.ShellIntegrationCheckBox.IsChecked = this.rememberedShellIntegration;
    }

    private void CloseButton_OnClick(object sender, RoutedEventArgs e) => this.bootstrapper.RequestClose();

    private void LaunchButton_OnClick(object sender, RoutedEventArgs e) => this.bootstrapper.RequestLaunchFabulor();

    private void OpenLogFolderButton_OnClick(object sender, RoutedEventArgs e)
    {
        Process.Start(new ProcessStartInfo(this.sessionLog.DirectoryPath) { UseShellExecute = true });
    }

    private void CopyErrorDetailsButton_OnClick(object sender, RoutedEventArgs e)
    {
        if (string.IsNullOrWhiteSpace(this.lastErrorDetails))
        {
            return;
        }

        try
        {
            System.Windows.Clipboard.SetText(this.lastErrorDetails);
            this.SetStatus("Error details copied to the clipboard.");
        }
        catch (COMException ex)
        {
            this.AppendLog($"Clipboard copy failed: {ex.Message}");
            this.SetStatus("Error details could not be copied. Try again in a moment.");
        }
    }

    private void LicenceButton_OnClick(object sender, RoutedEventArgs e)
    {
        var viewer = new System.Windows.Controls.RichTextBox
        {
            IsReadOnly = true,
            VerticalScrollBarVisibility = System.Windows.Controls.ScrollBarVisibility.Auto,
            HorizontalScrollBarVisibility = System.Windows.Controls.ScrollBarVisibility.Auto,
            Background = System.Windows.Media.Brushes.White,
            Foreground = System.Windows.Media.Brushes.Black,
            BorderThickness = new Thickness(0),
            Padding = new Thickness(12)
        };

        var resource = System.Windows.Application.GetResourceStream(new Uri("pack://application:,,,/Assets/Licence.rtf", UriKind.Absolute));
        if (resource is null)
        {
            System.Windows.MessageBox.Show(this, "The bundled licence text could not be loaded.", "Fabulor Setup", MessageBoxButton.OK, MessageBoxImage.Error);
            return;
        }

        using (resource.Stream)
        {
            viewer.Selection.Load(resource.Stream, System.Windows.DataFormats.Rtf);
        }

        var closeButton = new System.Windows.Controls.Button
        {
            Content = "Close",
            MinWidth = 96,
            Height = 32,
            Margin = new Thickness(0, 12, 0, 0),
            HorizontalAlignment = System.Windows.HorizontalAlignment.Right
        };
        var panel = new System.Windows.Controls.DockPanel { LastChildFill = true, Margin = new Thickness(14) };
        System.Windows.Controls.DockPanel.SetDock(closeButton, System.Windows.Controls.Dock.Bottom);
        panel.Children.Add(closeButton);
        panel.Children.Add(viewer);
        var dialog = new Window
        {
            Title = "Fabulor Licence",
            Owner = this,
            Width = 720,
            Height = 560,
            MinWidth = 520,
            MinHeight = 420,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            Content = panel
        };
        closeButton.Click += (_, _) => dialog.Close();
        dialog.ShowDialog();
    }

    private void InstallButton_OnClick(object sender, RoutedEventArgs e) => this.bootstrapper.RequestInstall();

    private void RepairButton_OnClick(object sender, RoutedEventArgs e) => this.bootstrapper.RequestRepair();

    private void UninstallButton_OnClick(object sender, RoutedEventArgs e) => this.bootstrapper.RequestUninstall();

    private void PortableModeCheckBox_OnChanged(object sender, RoutedEventArgs e)
    {
        if (!this.changingPortableModeProgrammatically && !this.installFolderEditedByUser)
        {
            this.InstallFolder = this.IsPortable
                ? this.portableDefaultFolder
                : this.installedDefaultFolder;
        }

        this.RefreshOptionState();
    }

    private void InstallFolderTextBox_OnTextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        if (this.trackingInstallFolderEdits && !this.changingInstallFolderProgrammatically)
        {
            this.installFolderEditedByUser = true;
        }

        if (string.IsNullOrWhiteSpace(this.InstallFolderTextBox.Text))
        {
            this.showingInstallFolderWarning = true;
            this.SetStatus("Choose an install folder before starting setup.");
            return;
        }

        if (this.showingInstallFolderWarning)
        {
            this.showingInstallFolderWarning = false;
            this.SetStatus(string.Empty);
        }
    }

    private void BrowseInstallFolderButton_OnClick(object sender, RoutedEventArgs e)
    {
        using var dialog = new Forms.FolderBrowserDialog
        {
            Description = "Choose an install folder for Fabulor",
            ShowNewFolderButton = true,
            SelectedPath = this.InstallFolder
        };
        if (dialog.ShowDialog() == Forms.DialogResult.OK && !string.IsNullOrWhiteSpace(dialog.SelectedPath))
        {
            this.installFolderEditedByUser = true;
            this.InstallFolder = dialog.SelectedPath;
            this.SetStatus($"Install folder set to {dialog.SelectedPath}");
        }
    }

    private static class NativeMethods
    {
        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool BringWindowToTop(IntPtr windowHandle);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool SetForegroundWindow(IntPtr windowHandle);
    }
}
