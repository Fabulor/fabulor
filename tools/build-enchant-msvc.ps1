param(
    [string]$GtkRoot = 'C:\gtk-build\gtk',
    [string]$StageRoot = 'C:\zoitechat-build\x64\rel',
    [string]$WorkRoot = (Join-Path $env:TEMP 'fabulor-enchant-msvc'),
    [string]$CMakePath = ''
)

$ErrorActionPreference = 'Stop'
$version = '2.8.19'
$archiveHash = 'C8D70991D544EE39274B96BD01D2858A009FE732FF43F2AAF605FD61ECD06F60'
$archive = Join-Path $WorkRoot "enchant-$version.tar.gz"
$source = Join-Path $WorkRoot "enchant-$version"
$build = Join-Path $WorkRoot 'build-msvc'
$install = Join-Path $WorkRoot 'install-msvc'
$recipe = Join-Path $PSScriptRoot 'enchant-msvc'

if ([string]::IsNullOrWhiteSpace($CMakePath)) {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCommand) {
        $CMakePath = $cmakeCommand.Source
    } else {
        $CMakePath = @(
            'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
        ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    }
}
if ([string]::IsNullOrWhiteSpace($CMakePath) -or -not (Test-Path -LiteralPath $CMakePath -PathType Leaf)) {
    throw 'CMake was not found. Install CMake, add cmake.exe to PATH, or pass -CMakePath.'
}

New-Item -ItemType Directory -Force -Path $WorkRoot | Out-Null
$archiveValid = (Test-Path -LiteralPath $archive) -and
    ((Get-FileHash -Algorithm SHA256 -LiteralPath $archive).Hash -eq $archiveHash)
for ($attempt = 1; -not $archiveValid -and $attempt -le 4; $attempt++) {
    Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
    try {
        Invoke-WebRequest -Uri "https://github.com/rrthomas/enchant/releases/download/v$version/enchant-$version.tar.gz" -OutFile $archive
        $archiveValid = (Get-FileHash -Algorithm SHA256 -LiteralPath $archive).Hash -eq $archiveHash
    } catch {
        $archiveValid = $false
    }
    if (-not $archiveValid -and $attempt -lt 4) {
        Start-Sleep -Seconds ([math]::Pow(2, $attempt))
    }
}
if (-not $archiveValid) {
    throw 'Enchant source archive failed SHA-256 verification after four download attempts.'
}
if (-not (Test-Path -LiteralPath $source)) {
    tar -xf $archive -C $WorkRoot
}

& $CMakePath -S $recipe -B $build -G 'Visual Studio 17 2022' -A x64 `
    "-DENCHANT_SOURCE_DIR=$source" "-DGTK_ROOT=$GtkRoot" "-DCMAKE_INSTALL_PREFIX=$install"
if ($LASTEXITCODE -ne 0) { throw 'Enchant CMake configuration failed.' }
& $CMakePath --build $build --config Release --target install
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
