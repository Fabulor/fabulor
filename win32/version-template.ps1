param ([string] $templateFilename, [string] $outputFilename)

$repoRoot = $env:SOLUTIONDIR
$versionParts = $null

$mesonVersionFile = Join-Path $repoRoot 'meson.build'
if (Test-Path $mesonVersionFile)
{
	$versionParts = Select-String -Path $mesonVersionFile -Pattern "  version: '([^']+)',$" | Select-Object -First 1 | %{ $_.Matches[0].Groups[1].Value.Split('.') }
}

if (-not $versionParts)
{
	$installerPropsPath = Join-Path $repoRoot 'installer\Directory.Build.props'
	if (-not (Test-Path $installerPropsPath))
	{
		throw "Unable to resolve version source. Neither '$mesonVersionFile' nor '$installerPropsPath' exists."
	}

	[xml] $installerProps = Get-Content $installerPropsPath -Encoding UTF8
	$semanticVersion = $installerProps.Project.PropertyGroup.FabulorSemVer
	if ([string]::IsNullOrWhiteSpace($semanticVersion))
	{
		$semanticVersion = $installerProps.Project.PropertyGroup.ZoiteChatSemVer
	}
	if ([string]::IsNullOrWhiteSpace($semanticVersion))
	{
		throw "Unable to resolve FabulorSemVer or ZoiteChatSemVer from '$installerPropsPath'."
	}

	$versionParts = $semanticVersion.Split('.')
}

[string[]] $contents = Get-Content $templateFilename -Encoding UTF8 | %{
	while ($_ -match '^(.*?)<#=(.*?)#>(.*?)$') {
		$_ = $Matches[1] + $(Invoke-Expression $Matches[2]) + $Matches[3]
	}
	$_
}

[System.IO.File]::WriteAllLines($outputFilename, $contents)
