param(
    [Parameter(Mandatory)]
    [string] $Bootstrapper,

    [Parameter(Mandatory)]
    [string] $VersionProps
)

$ErrorActionPreference = 'Stop'
if (-not $env:RUNNER_TEMP) { throw 'Run this test on a disposable Windows CI runner.' }
$setup = (Resolve-Path -LiteralPath $Bootstrapper).Path
$properties = [xml](Get-Content -LiteralPath $VersionProps -Raw)
$version = [string]$properties.Project.PropertyGroup.FabulorSemVer
$expectedTitle = "Fabulor Setup $version"
$installFolder = Join-Path $env:ProgramFiles 'Fabulor'
$logFolder = Join-Path $env:RUNNER_TEMP 'fabulor-setup-install-mode-logs'
$uninstallRegistry = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall'

function Get-FabulorSetupEntries {
    Get-ChildItem -LiteralPath $uninstallRegistry |
        ForEach-Object { Get-ItemProperty -LiteralPath $_.PSPath } |
        Where-Object { $_.DisplayName -eq 'Fabulor Setup' }
}

function Assert-CleanRunner {
    if ((Test-Path -LiteralPath $installFolder) -or @(Get-FabulorSetupEntries).Count) {
        throw 'The install-mode test requires a disposable runner without Fabulor installed.'
    }
}

function Invoke-FabulorSetup([string] $Name, [string] $Arguments) {
    $log = Join-Path $logFolder "$Name.log"
    $startArguments = "$Arguments /norestart /log " + '"' + $log + '"'
    $process = Start-Process -FilePath $setup -ArgumentList $startArguments -PassThru
    $windowTitles = [Collections.Generic.HashSet[string]]::new()
    $deadline = [DateTime]::UtcNow.AddMinutes(4)
    do {
        $windows = Get-Process -Name 'Fabulor.BA' -ErrorAction SilentlyContinue |
            Where-Object { $_.MainWindowHandle -ne 0 }
        foreach ($window in $windows) {
            [void]$windowTitles.Add($window.MainWindowTitle)
        }
        if ([DateTime]::UtcNow -gt $deadline) {
            throw "Setup did not exit within four minutes for $Name. See $log."
        }
        Start-Sleep -Milliseconds 125
        $process.Refresh()
    } until ($process.HasExited)

    Write-Host "$Name returned $($process.ExitCode). Setup window titles: $($windowTitles -join ', ')"
    [pscustomobject]@{
        ExitCode = $process.ExitCode
        WindowTitles = $windowTitles
        Log = Get-Content -LiteralPath $log -Raw
    }
}

Assert-CleanRunner
New-Item -Path $logFolder -ItemType Directory -Force | Out-Null

foreach ($mode in @('quiet', 'passive')) {
    $result = Invoke-FabulorSetup -Name "install-$mode" -Arguments "/install /$mode"
    $display = if ($mode -eq 'quiet') { 'None' } else { 'Passive' }
    if ($result.ExitCode -ne 0 -or
        $result.Log -notmatch "display=$display, nonInteractive=True," -or
        $result.Log -notmatch 'Apply complete, result: 0x0,') {
        throw "The /$mode installer did not complete successfully."
    }

    $entries = @(Get-FabulorSetupEntries)
    if ($entries.Count -ne 1 -or $entries[0].DisplayVersion -ne $version -or
        -not (Test-Path -LiteralPath (Join-Path $installFolder 'fabulor.exe'))) {
        throw "The /$mode installer did not create the expected installation."
    }
    if ($mode -eq 'quiet' -and $result.WindowTitles.Count) {
        throw 'Quiet mode showed a Setup window.'
    }
    if ($mode -eq 'passive' -and -not $result.WindowTitles.Contains($expectedTitle)) {
        throw "Passive mode did not show a window titled '$expectedTitle'."
    }

    $removal = Invoke-FabulorSetup -Name "uninstall-after-$mode" -Arguments '/uninstall /quiet'
    if ($removal.ExitCode -ne 0 -or $removal.WindowTitles.Count) {
        throw "Quiet uninstall failed or displayed a window after /$mode install."
    }
    Assert-CleanRunner
}

$invalidArguments = '/install /quiet FABULOR_PORTABLE=1 InstallFolder="{0}"' -f $installFolder
$invalid = Invoke-FabulorSetup -Name 'reject-protected-portable-location' -Arguments $invalidArguments
if ($invalid.ExitCode -eq 0 -or $invalid.WindowTitles.Count -or
    $invalid.Log -notmatch 'Portable mode cannot use a protected Windows folder') {
    throw 'Quiet Setup did not reject a protected portable location without showing a window.'
}
Assert-CleanRunner
