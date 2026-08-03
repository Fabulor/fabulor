param(
    [Parameter(Mandatory = $true)] [string] $Launcher,
    [Parameter(Mandatory = $true)] [string] $Frontend,
    [Parameter(Mandatory = $true)] [string] $Bootstrapper,
    [Parameter(Mandatory = $true)] [string] $BootstrapperApplication,
    [Parameter(Mandatory = $true)] [string] $VersionProps
)

$ErrorActionPreference = 'Stop'

function Require-Value {
    param([string] $Label, [object] $Actual, [object] $Expected)
    if ($Actual -ne $Expected) {
        throw "$Label mismatch: expected '$Expected', found '$Actual'."
    }
}

function Get-Metadata {
    param([string] $Path)
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    return [Diagnostics.FileVersionInfo]::GetVersionInfo($resolved)
}

[xml] $props = Get-Content -LiteralPath (Resolve-Path -LiteralPath $VersionProps)
$group = $props.Project.PropertyGroup
$product = [string] $group.FabulorProductName
$publisher = [string] $group.FabulorPublisher
$semver = [string] $group.FabulorSemVer
$parts = @($semver.Split('.') | ForEach-Object { [int] $_ })
if ($parts.Count -ne 3) {
    throw 'FabulorSemVer must contain exactly three numeric parts.'
}

$contracts = @(
    @{
        Path = $Launcher; Label = 'launcher'; Description = 'Fabulor IRC Client';
        Product = $product; Company = $publisher; Original = 'fabulor.exe'
    },
    @{
        Path = $Frontend; Label = 'frontend'; Description = 'Fabulor GTK4 Frontend';
        Product = $product; Company = $publisher; Original = 'fabulor-gtk4-frontend.dll'
    },
    @{
        Path = $Bootstrapper; Label = 'bootstrapper'; Description = 'Fabulor Setup';
        Product = 'Fabulor Setup'; Company = $publisher; Original = 'FabulorSetup.exe'
    },
    @{
        Path = $BootstrapperApplication; Label = 'bootstrapper application';
        Description = 'Fabulor Windows Installer'; Product = $product;
        Company = $publisher; Original = 'Fabulor.BA.dll'
    }
)

foreach ($contract in $contracts) {
    $metadata = Get-Metadata $contract.Path
    Require-Value "$($contract.Label) FileDescription" $metadata.FileDescription $contract.Description
    Require-Value "$($contract.Label) ProductName" $metadata.ProductName $contract.Product
    Require-Value "$($contract.Label) CompanyName" $metadata.CompanyName $contract.Company
    Require-Value "$($contract.Label) OriginalFilename" $metadata.OriginalFilename $contract.Original
    Require-Value "$($contract.Label) FileMajorPart" $metadata.FileMajorPart $parts[0]
    Require-Value "$($contract.Label) FileMinorPart" $metadata.FileMinorPart $parts[1]
    Require-Value "$($contract.Label) FileBuildPart" $metadata.FileBuildPart $parts[2]
    Require-Value "$($contract.Label) FilePrivatePart" $metadata.FilePrivatePart 0
    if ($metadata.ProductVersion -ne $semver) {
        throw "$($contract.Label) ProductVersion mismatch: expected '$semver', found '$($metadata.ProductVersion)'."
    }
}

Write-Output "Product metadata validated: product=$product publisher=$publisher version=$semver files=$($contracts.Count)"
