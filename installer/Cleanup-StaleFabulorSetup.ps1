#Requires -RunAsAdministrator

param(
    [switch]$RemoveAll
)

$ErrorActionPreference = 'Stop'

$uninstallRoot = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall'
$dependencyRoot = 'HKLM:\SOFTWARE\Classes\Installer\Dependencies'
$packageCacheRoot = 'C:\ProgramData\Package Cache'
$bundleUpgradeCode = 'D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10'

$removed = 0
$preservedBundleCachePath = $null
$preservedBundleCode = $null
$preservedMsiProductCode = $null

function Test-SamePath {
    param(
        [Parameter(Mandatory = $true)][string]$Left,
        [Parameter(Mandatory = $true)][string]$Right
    )

    try {
        return [string]::Equals(
            [System.IO.Path]::GetFullPath($Left),
            [System.IO.Path]::GetFullPath($Right),
            [System.StringComparison]::OrdinalIgnoreCase)
    }
    catch {
        return [string]::Equals($Left, $Right, [System.StringComparison]::OrdinalIgnoreCase)
    }
}

function Test-FabulorSetupBundleEntry {
    param(
        [Parameter(Mandatory = $true)]$Props
    )

    return $Props.DisplayName -eq 'Fabulor Setup' -and (
        $Props.BundleProviderKey -eq 'Fabulor.Setup.Bundle' -or
        ($Props.BundleUpgradeCode -and $Props.BundleUpgradeCode -like "*$bundleUpgradeCode*"))
}

if (-not $RemoveAll -and (Test-Path -LiteralPath $uninstallRoot)) {
    $setupEntries = Get-ChildItem -LiteralPath $uninstallRoot | ForEach-Object {
        $props = Get-ItemProperty -LiteralPath $_.PSPath
        if (Test-FabulorSetupBundleEntry $props) {
            [pscustomobject]@{
                Key = $_
                BundleCachePath = $props.BundleCachePath
                LastWriteTime = if ($props.BundleCachePath -and (Test-Path -LiteralPath $props.BundleCachePath)) {
                    (Get-Item -LiteralPath $props.BundleCachePath).LastWriteTimeUtc
                } else {
                    [datetime]::MinValue
                }
            }
        }
    }

    $preservedBundleCachePath = $setupEntries |
        Where-Object { $_.BundleCachePath -and (Test-Path -LiteralPath $_.BundleCachePath) } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1 -ExpandProperty BundleCachePath

    if ($preservedBundleCachePath) {
        $parent = [System.IO.Path]::GetDirectoryName($preservedBundleCachePath)
        $candidateCode = [System.IO.Path]::GetFileName($parent)
        if ($candidateCode -match '^\{[0-9A-Fa-f-]{36}\}$') {
            $preservedBundleCode = $candidateCode
        }
    }

    $preservedMsiProductCode = Get-ChildItem -LiteralPath $uninstallRoot | ForEach-Object {
        $props = Get-ItemProperty -LiteralPath $_.PSPath
        if ($props.DisplayName -eq 'Fabulor' -and $props.SystemComponent -eq 1) {
            $_.PSChildName
        }
    } | Select-Object -First 1
}

if (Test-Path -LiteralPath $uninstallRoot) {
    Get-ChildItem -LiteralPath $uninstallRoot | ForEach-Object {
        $props = Get-ItemProperty -LiteralPath $_.PSPath
        if (Test-FabulorSetupBundleEntry $props) {
            if (-not $RemoveAll -and $preservedBundleCachePath -and $props.BundleCachePath -and (Test-SamePath $props.BundleCachePath $preservedBundleCachePath)) {
                Write-Host "Preserved current uninstall registration: $($_.PSChildName)"
            } else {
                Remove-Item -LiteralPath $_.PSPath -Recurse -Force
                Write-Host "Removed stale uninstall registration: $($_.PSChildName)"
                $script:removed++
            }
        }
    }
}

if (Test-Path -LiteralPath $dependencyRoot) {
    $bundleDependents = Join-Path $dependencyRoot 'Fabulor.Setup.Bundle\Dependents'
    if (Test-Path -LiteralPath $bundleDependents) {
        Get-ChildItem -LiteralPath $bundleDependents | ForEach-Object {
            if (-not $RemoveAll -and $preservedBundleCode -and $_.PSChildName -eq $preservedBundleCode) {
                Write-Host "Preserved current bundle dependent: $($_.PSChildName)"
            } else {
                Remove-Item -LiteralPath $_.PSPath -Recurse -Force
                Write-Host "Removed stale bundle dependent: $($_.PSChildName)"
                $script:removed++
            }
        }
    }

    Get-ChildItem -LiteralPath $dependencyRoot | ForEach-Object {
        $props = Get-ItemProperty -LiteralPath $_.PSPath
        $displayName = $props.DisplayName
        $isCurrentMsiDependency = -not $RemoveAll -and $preservedMsiProductCode -and $_.PSChildName.StartsWith($preservedMsiProductCode, [System.StringComparison]::OrdinalIgnoreCase)
        $isCurrentBundleDependency = -not $RemoveAll -and (
            $_.PSChildName -eq 'Fabulor.Setup.Bundle' -or
            ($preservedBundleCode -and $_.PSChildName -eq $preservedBundleCode))

        if ($isCurrentMsiDependency -or $isCurrentBundleDependency) {
            Write-Host "Preserved current dependency registration: $($_.PSChildName)"
        } elseif ($_.PSChildName -eq 'Fabulor.Setup.Bundle' -or $displayName -eq 'Fabulor Setup' -or $displayName -eq 'Fabulor') {
            Remove-Item -LiteralPath $_.PSPath -Recurse -Force
            Write-Host "Removed stale dependency registration: $($_.PSChildName)"
            $script:removed++
        }
    }
}

if (Test-Path -LiteralPath $packageCacheRoot) {
    $packageCacheRootPath = [System.IO.Path]::GetFullPath($packageCacheRoot)
    Get-ChildItem -LiteralPath $packageCacheRoot -Recurse -Filter FabulorSetup.exe -ErrorAction SilentlyContinue | ForEach-Object {
        if (-not $RemoveAll -and $preservedBundleCachePath -and (Test-SamePath $_.FullName $preservedBundleCachePath)) {
            Write-Host "Preserved current package cache: $($_.FullName)"
        } else {
            $cacheDirectory = $_.Directory.FullName
            $cacheDirectoryPath = [System.IO.Path]::GetFullPath($cacheDirectory)
            if (-not $cacheDirectoryPath.StartsWith($packageCacheRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
                throw "Refusing to remove package cache outside ${packageCacheRoot}: $cacheDirectory"
            }

            Remove-Item -LiteralPath $cacheDirectory -Recurse -Force
            Write-Host "Removed stale package cache: $cacheDirectory"
            $script:removed++
        }
    }
}

Write-Host "Removed $removed stale Fabulor setup item$(if ($removed -eq 1) { '' } else { 's' })."
