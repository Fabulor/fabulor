using System.Text;
using System.Windows;
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
        IncludeThemeAssets = this.ThemeAssetsCheckBox.IsChecked == true,
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
        this.ThemeAssetsCheckBox.IsChecked = selection.IncludeThemeAssets;
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
        this.ThemeAssetsCheckBox.IsEnabled = !isBusy;
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

    public void SetDetectedState(bool isInstalled)
    {
        this.InstallButton.Content = isInstalled ? "Modify" : "Install";
        this.RepairButton.IsEnabled = isInstalled;
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
}
