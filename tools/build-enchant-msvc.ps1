param(
    [string]$GtkRoot = 'C:\gtk-build\gtk',
    [string]$StageRoot = 'C:\zoitechat-build\x64\rel',
    [string]$WorkRoot = (Join-Path $env:TEMP 'fabulor-enchant-msvc')
)

$ErrorActionPreference = 'Stop'
$version = '2.8.19'
$archiveHash = '8E7F6CB0C3B79BE3146EB3AB93650484ADBC59DAE5F2C1958FDE557080BA678C'
$archive = Join-Path $WorkRoot "enchant-$version.tar.gz"
$source = Join-Path $WorkRoot "enchant-$version"
$build = Join-Path $WorkRoot 'build-msvc'
$install = Join-Path $WorkRoot 'install-msvc'
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$recipe = Join-Path $PSScriptRoot 'enchant-msvc'

New-Item -ItemType Directory -Force -Path $WorkRoot | Out-Null
if (-not (Test-Path -LiteralPath $archive)) {
    Invoke-WebRequest -Uri "https://github.com/rrthomas/enchant/releases/download/v$version/enchant-$version.tar.gz" -OutFile $archive
}
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $archive).Hash -ne $archiveHash) {
    throw 'Enchant source archive hash mismatch.'
}
if (-not (Test-Path -LiteralPath $source)) {
    tar -xf $archive -C $WorkRoot
}

& $cmake -S $recipe -B $build -G 'Visual Studio 17 2022' -A x64 `
    "-DENCHANT_SOURCE_DIR=$source" "-DGTK_ROOT=$GtkRoot" "-DCMAKE_INSTALL_PREFIX=$install"
if ($LASTEXITCODE -ne 0) { throw 'Enchant CMake configuration failed.' }
& $cmake --build $build --config Release --target install
if ($LASTEXITCODE -ne 0) { throw 'Enchant MSVC build failed.' }

New-Item -ItemType Directory -Force -Path (Join-Path $StageRoot 'lib\enchant-2') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $StageRoot 'share\enchant-2') | Out-Null
Copy-Item -Force -LiteralPath (Join-Path $install 'libenchant-2-2.dll') -Destination $StageRoot
Copy-Item -Force -LiteralPath (Join-Path $install 'lib\enchant-2\enchant_winspell.dll') -Destination (Join-Path $StageRoot 'lib\enchant-2')
Copy-Item -Force -LiteralPath (Join-Path $install 'share\enchant-2\enchant.ordering') -Destination (Join-Path $StageRoot 'share\enchant-2')

$smokeConfig = Join-Path $WorkRoot 'smoke-config'
New-Item -ItemType Directory -Force -Path $smokeConfig | Out-Null
$env:XDG_CONFIG_HOME = $smokeConfig
$env:PATH = "$StageRoot;$env:PATH"
& (Join-Path $install 'enchant_smoke.exe')
if ($LASTEXITCODE -ne 0) { throw "Enchant smoke test failed with exit code $LASTEXITCODE." }

Get-FileHash -Algorithm SHA256 -LiteralPath `
    (Join-Path $StageRoot 'libenchant-2-2.dll'), `
    (Join-Path $StageRoot 'lib\enchant-2\enchant_winspell.dll')
