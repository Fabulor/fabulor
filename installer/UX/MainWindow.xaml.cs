using System.Text;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using Forms = global::System.Windows.Forms;

namespace Fabulor.Setup;

public partial class MainWindow : Window
{
    private readonly FabulorBootstrapperApplication bootstrapper;
    private bool rememberedStartMenuShortcuts = true;
    private bool rememberedShellIntegration = true;

    public MainWindow(FabulorBootstrapperApplication bootstrapper)
    {
        this.bootstrapper = bootstrapper;
        this.InitializeComponent();
        this.UpdateModeSummary();
    }

    public string InstallFolder
    {
        get => this.InstallFolderTextBox.Text.Trim();
        set => this.InstallFolderTextBox.Text = value;
    }

    public bool IsPortable => this.PortableModeCheckBox.IsChecked == true;

    public InstallerFeatureSelection FeatureSelection => new()
    {
        IncludeDotNetPluginHost = this.DotNetPluginHostCheckBox.IsChecked == true,
        IncludePythonRuntime = this.PythonRuntimeCheckBox.IsChecked == true,
        IncludeTclRuntime = this.TclRuntimeCheckBox.IsChecked == true,
        IncludeStartMenuShortcuts = this.StartMenuShortcutsCheckBox.IsChecked == true,
        IncludeShellIntegration = this.ShellIntegrationCheckBox.IsChecked == true,
        IncludeTranslations = this.TranslationsCheckBox.IsChecked == true,
        IncludeChecksumPlugin = this.ChecksumPluginCheckBox.IsChecked == true,
        IncludeExecPlugin = this.ExecPluginCheckBox.IsChecked == true,
        IncludeFishlimPlugin = this.FishlimPluginCheckBox.IsChecked == true,
        IncludeSysinfoPlugin = this.SysinfoPluginCheckBox.IsChecked == true,
        IncludeUpdatePlugin = this.UpdatePluginCheckBox.IsChecked == true
    };

    public void SetPortableMode(bool isPortable)
    {
        this.PortableModeCheckBox.IsChecked = isPortable;
        this.RefreshOptionState();
    }

    public void SetFeatureSelection(InstallerFeatureSelection selection)
    {
        this.rememberedStartMenuShortcuts = selection.IncludeStartMenuShortcuts;
        this.rememberedShellIntegration = selection.IncludeShellIntegration;

        this.DotNetPluginHostCheckBox.IsChecked = selection.IncludeDotNetPluginHost;
        this.PythonRuntimeCheckBox.IsChecked = selection.IncludePythonRuntime;
        this.TclRuntimeCheckBox.IsChecked = selection.IncludeTclRuntime;
        this.StartMenuShortcutsCheckBox.IsChecked = selection.IncludeStartMenuShortcuts;
        this.ShellIntegrationCheckBox.IsChecked = selection.IncludeShellIntegration;
        this.TranslationsCheckBox.IsChecked = selection.IncludeTranslations;
        this.ChecksumPluginCheckBox.IsChecked = selection.IncludeChecksumPlugin;
        this.ExecPluginCheckBox.IsChecked = selection.IncludeExecPlugin;
        this.FishlimPluginCheckBox.IsChecked = selection.IncludeFishlimPlugin;
        this.SysinfoPluginCheckBox.IsChecked = selection.IncludeSysinfoPlugin;
        this.UpdatePluginCheckBox.IsChecked = selection.IncludeUpdatePlugin;
        this.RefreshOptionState();
    }

    public void AppendLog(string message)
    {
        if (string.IsNullOrWhiteSpace(message))
        {
            return;
        }

        var builder = new StringBuilder(this.LogTextBox.Text);
        if (builder.Length > 0)
        {
            builder.AppendLine();
        }

        builder.Append(message);
        this.LogTextBox.Text = builder.ToString();
        this.LogTextBox.ScrollToEnd();
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
        this.UpdatePluginCheckBox.IsEnabled = !isBusy;

        if (isBusy)
        {
            this.StartMenuShortcutsCheckBox.IsEnabled = false;
            this.ShellIntegrationCheckBox.IsEnabled = false;
            return;
        }

        this.RefreshOptionState();
    }

    public void SetDetectedState(bool isInstalled, bool isCurrentPackageInstalled)
    {
        this.InstallButton.Content = isInstalled
            ? isCurrentPackageInstalled ? "Modify" : "Upgrade"
            : "Install";
        this.RepairButton.IsEnabled = isCurrentPackageInstalled;
        this.UninstallButton.IsEnabled = isInstalled;
    }

    public void SetProgress(int percentage)
    {
        this.ProgressBar.Value = percentage < 0 ? 0 : percentage > 100 ? 100 : percentage;
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

            if (handle != IntPtr.Zero)
            {
                activated = NativeMethods.SetForegroundWindow(handle) || activated;
            }
        }

        this.Focus();
        return activated;
    }

    private void UpdateModeSummary()
    {
        this.ModeSummaryTextBlock.Text = this.IsPortable
            ? "Portable mode keeps configuration beside the executable and suppresses installed-mode registry integration."
            : "Installed mode writes Start menu and protocol/theme registration through the MSI and stores configuration under the roaming profile.";
    }

    private void RefreshOptionState()
    {
        this.UpdateModeSummary();

        if (this.IsPortable)
        {
            this.rememberedStartMenuShortcuts = this.StartMenuShortcutsCheckBox.IsChecked == true;
            this.rememberedShellIntegration = this.ShellIntegrationCheckBox.IsChecked == true;
            this.StartMenuShortcutsCheckBox.IsChecked = false;
            this.ShellIntegrationCheckBox.IsChecked = false;
            this.StartMenuShortcutsCheckBox.IsEnabled = false;
            this.ShellIntegrationCheckBox.IsEnabled = false;
            return;
        }

        this.StartMenuShortcutsCheckBox.IsEnabled = true;
        this.ShellIntegrationCheckBox.IsEnabled = true;
        this.StartMenuShortcutsCheckBox.IsChecked = this.rememberedStartMenuShortcuts;
        this.ShellIntegrationCheckBox.IsChecked = this.rememberedShellIntegration;
    }

    private void CloseButton_OnClick(object sender, RoutedEventArgs e)
    {
        this.bootstrapper.RequestClose();
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

        var licenceUri = new Uri("pack://application:,,,/Assets/Licence.rtf", UriKind.Absolute);
        var resource = System.Windows.Application.GetResourceStream(licenceUri);
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

        var panel = new System.Windows.Controls.DockPanel
        {
            LastChildFill = true,
            Margin = new Thickness(14)
        };

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

    private void InstallButton_OnClick(object sender, RoutedEventArgs e)
    {
        this.bootstrapper.RequestInstall();
    }

    private void RepairButton_OnClick(object sender, RoutedEventArgs e)
    {
        this.bootstrapper.RequestRepair();
    }

    private void UninstallButton_OnClick(object sender, RoutedEventArgs e)
    {
        this.bootstrapper.RequestUninstall();
    }

    private void PortableModeCheckBox_OnChanged(object sender, RoutedEventArgs e)
    {
        this.RefreshOptionState();
    }

    private void InstallFolderTextBox_OnTextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        if (!string.IsNullOrWhiteSpace(this.InstallFolderTextBox.Text))
        {
            return;
        }

        this.SetStatus("Choose an install folder before starting a bundle action.");
    }

    private void BrowseInstallFolderButton_OnClick(object sender, RoutedEventArgs e)
    {
        using var dialog = new Forms.FolderBrowserDialog
        {
            Description = "Choose an install folder for Fabulor",
            ShowNewFolderButton = true
        };

        if (!string.IsNullOrWhiteSpace(this.InstallFolder))
        {
            dialog.SelectedPath = this.InstallFolder;
        }

        if (dialog.ShowDialog() == Forms.DialogResult.OK && !string.IsNullOrWhiteSpace(dialog.SelectedPath))
        {
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
