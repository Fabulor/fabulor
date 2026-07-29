[CmdletBinding()]
param (
	[Parameter(Mandatory = $true)]
	[ValidateNotNullOrEmpty()]
	[string] $templateFilename,

	[Parameter(Mandatory = $true)]
	[ValidateNotNullOrEmpty()]
	[string] $outputFilename
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($env:SOLUTIONDIR))
{
	throw 'SOLUTIONDIR is not set.'
}
if (-not (Test-Path -LiteralPath $templateFilename -PathType Leaf))
{
	throw "Version template was not found: $templateFilename"
}

$repoRoot = [System.IO.Path]::GetFullPath($env:SOLUTIONDIR)
$versionParts = $null

$installerPropsPath = Join-Path $repoRoot 'installer\Directory.Build.props'
if (Test-Path -LiteralPath $installerPropsPath -PathType Leaf)
{
	[xml] $installerProps = Get-Content -LiteralPath $installerPropsPath -Encoding UTF8
	$semanticVersion = $installerProps.Project.PropertyGroup.FabulorSemVer
	if ([string]::IsNullOrWhiteSpace($semanticVersion))
	{
		$semanticVersion = $installerProps.Project.PropertyGroup.ZoiteChatSemVer
	}
	if (-not [string]::IsNullOrWhiteSpace($semanticVersion))
	{
		$versionParts = $semanticVersion.Split('.')
	}
}

if (-not $versionParts)
{
	throw "Unable to resolve FabulorSemVer from installer\Directory.Build.props."
}

[string[]] $contents = Get-Content -LiteralPath $templateFilename -Encoding UTF8 | ForEach-Object {
	while ($_ -match '^(.*?)<#=(.*?)#>(.*?)$') {
		$_ = $Matches[1] + $(Invoke-Expression $Matches[2]) + $Matches[3]
	}
	$_
}

if ($contents.Count -eq 0)
{
	throw "Version template is empty: $templateFilename"
}

$outputPath = [System.IO.Path]::GetFullPath($outputFilename)
$outputDirectory = [System.IO.Path]::GetDirectoryName($outputPath)
if (-not (Test-Path -LiteralPath $outputDirectory -PathType Container))
{
	throw "Version template output directory does not exist: $outputDirectory"
}

[System.IO.File]::WriteAllLines($outputPath, $contents)
