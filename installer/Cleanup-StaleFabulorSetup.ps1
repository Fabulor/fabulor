#Requires -RunAsAdministrator

$ErrorActionPreference = 'Stop'

$uninstallRoot = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall'
$dependencyRoot = 'HKLM:\SOFTWARE\Classes\Installer\Dependencies'

$removed = 0

Get-ChildItem -LiteralPath $uninstallRoot | ForEach-Object {
    $props = Get-ItemProperty -LiteralPath $_.PSPath
    if ($props.DisplayName -eq 'Fabulor Setup' -and $props.BundleProviderKey -eq 'Fabulor.Setup.Bundle') {
        Remove-Item -LiteralPath $_.PSPath -Recurse -Force
        Write-Host "Removed uninstall registration: $($_.PSChildName)"
        $script:removed++
    }
}

if (Test-Path -LiteralPath $dependencyRoot) {
    Get-ChildItem -LiteralPath $dependencyRoot | ForEach-Object {
        $props = Get-ItemProperty -LiteralPath $_.PSPath
        $displayName = $props.DisplayName
        if ($_.PSChildName -eq 'Fabulor.Setup.Bundle' -or $displayName -eq 'Fabulor Setup' -or $displayName -eq 'Fabulor') {
            Remove-Item -LiteralPath $_.PSPath -Recurse -Force
            Write-Host "Removed dependency registration: $($_.PSChildName)"
            $script:removed++
        }
    }
}

Write-Host "Removed $removed stale Fabulor setup registry entr$(if ($removed -eq 1) { 'y' } else { 'ies' })."
