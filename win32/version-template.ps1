param ([string] $templateFilename, [string] $outputFilename)

$repoRoot = $env:SOLUTIONDIR
$versionParts = $null

$installerPropsPath = Join-Path $repoRoot 'installer\Directory.Build.props'
if (Test-Path $installerPropsPath)
{
	[xml] $installerProps = Get-Content $installerPropsPath -Encoding UTF8
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
	$mesonVersionFile = Join-Path $repoRoot 'meson.build'
	if (Test-Path $mesonVersionFile)
	{
		$versionParts = Select-String -Path $mesonVersionFile -Pattern "  version: '([^']+)',$" | Select-Object -First 1 | %{ $_.Matches[0].Groups[1].Value.Split('.') }
	}
}

if (-not $versionParts)
{
	throw "Unable to resolve version source. Neither installer semver nor meson.build version could be read."
}

[string[]] $contents = Get-Content $templateFilename -Encoding UTF8 | %{
	while ($_ -match '^(.*?)<#=(.*?)#>(.*?)$') {
		$_ = $Matches[1] + $(Invoke-Expression $Matches[2]) + $Matches[3]
	}
	$_
}

[System.IO.File]::WriteAllLines($outputFilename, $contents)
