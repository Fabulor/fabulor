[CmdletBinding()]
param(
    [switch]$Apply,
    [switch]$IncludeInstallerArtifacts
)

$ErrorActionPreference = "Stop"

function Get-CanonicalPath {
    param([Parameter(Mandatory)][string]$Path)

    return [System.IO.Path]::GetFullPath($Path).TrimEnd(
        [System.IO.Path]::DirectorySeparatorChar,
        [System.IO.Path]::AltDirectorySeparatorChar
    )
}

$repoRoot = Get-CanonicalPath (Join-Path $PSScriptRoot "..")
$repoPrefix = $repoRoot + [System.IO.Path]::DirectorySeparatorChar

if (-not (Test-Path -LiteralPath (Join-Path $repoRoot ".git") -PathType Container)) {
    throw "Cleanup must run from a copy of this script inside a Fabulor Git worktree."
}

$protectedRelativePaths = @(
    ".git",
    ".vscode",
    "Runtime",
    "dos2unix.exe"
)
$protectedPaths = $protectedRelativePaths | ForEach-Object {
    Get-CanonicalPath (Join-Path $repoRoot $_)
}

$candidateMap = @{}

function Add-CleanupCandidate {
    param([Parameter(Mandatory)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }

    $canonical = Get-CanonicalPath $Path
    if ($canonical -eq $repoRoot -or
        -not $canonical.StartsWith($repoPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing cleanup target outside the repository: $canonical"
    }

    foreach ($protected in $protectedPaths) {
        if ($canonical -eq $protected -or
            $canonical.StartsWith(
                $protected + [System.IO.Path]::DirectorySeparatorChar,
                [System.StringComparison]::OrdinalIgnoreCase
            )) {
            return
        }
    }

    $candidateMap[$canonical] = $true
}

$fixedIntermediatePaths = @(
    "build",
    "builddir",
    "html",
    ".vs",
    "installer\obj",
    "installer\build",
    "installer\Bootstrapper\obj",
    "installer\UX\obj",
    "installer\bootstrapper.log",
    "tags"
)

foreach ($relativePath in $fixedIntermediatePaths) {
    Add-CleanupCandidate (Join-Path $repoRoot $relativePath)
}

foreach ($pattern in @("doxygen*.tmp", "*.wixpdb")) {
    Get-ChildItem -LiteralPath $repoRoot -Filter $pattern -File -Recurse `
        -Force -ErrorAction SilentlyContinue |
        ForEach-Object { Add-CleanupCandidate $_.FullName }
}

foreach ($searchRoot in @("src\managed", "samples")) {
    $absoluteRoot = Join-Path $repoRoot $searchRoot
    if (Test-Path -LiteralPath $absoluteRoot -PathType Container) {
        Get-ChildItem -LiteralPath $absoluteRoot -Directory -Recurse `
            -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -in @("bin", "obj") } |
            ForEach-Object { Add-CleanupCandidate $_.FullName }
    }
}

Get-ChildItem -LiteralPath $repoRoot -Directory -Filter "__pycache__" -Recurse `
    -Force -ErrorAction SilentlyContinue |
    ForEach-Object { Add-CleanupCandidate $_.FullName }

if ($IncludeInstallerArtifacts) {
    foreach ($relativePath in @("installer\bin", "installer\UX\bin")) {
        Add-CleanupCandidate (Join-Path $repoRoot $relativePath)
    }
}

$candidates = $candidateMap.Keys |
    Sort-Object { $_.Length } |
    Where-Object {
        $candidate = $_
        -not ($candidateMap.Keys | Where-Object {
            $_ -ne $candidate -and
            $candidate.StartsWith(
                $_ + [System.IO.Path]::DirectorySeparatorChar,
                [System.StringComparison]::OrdinalIgnoreCase
            )
        })
    }

Write-Host "Fabulor development-output cleanup"
Write-Host "Repository: $repoRoot"
Write-Host ("Mode: " + $(if ($Apply) { "apply" } else { "preview" }))
Write-Host "Protected: Runtime, .vscode, dos2unix.exe, .git, tracked source, and every path outside this worktree"

if (-not $candidates) {
    Write-Host "No matching development output was found."
    exit 0
}

foreach ($candidate in $candidates) {
    Write-Host ("  " + $(if ($Apply) { "REMOVE " } else { "WOULD REMOVE " }) + $candidate)
}

if (-not $Apply) {
    Write-Host "Preview only. Re-run with -Apply to remove the listed output."
    exit 0
}

foreach ($candidate in $candidates) {
    $item = Get-Item -LiteralPath $candidate -Force
    if ($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
        throw "Refusing cleanup target that is a reparse point: $candidate"
    }

    if ($item.PSIsContainer) {
        $enumerationErrors = @()
        $reparsePoints = @(
            Get-ChildItem -LiteralPath $candidate -Recurse -Force `
                -ErrorAction SilentlyContinue -ErrorVariable +enumerationErrors |
                Where-Object {
                    $_.Attributes -band [System.IO.FileAttributes]::ReparsePoint
                }
        )
        if ($enumerationErrors.Count -gt 0) {
            throw "Refusing to delete an output tree that could not be fully inspected: $candidate"
        }
        if ($reparsePoints.Count -gt 0) {
            throw "Refusing to delete an output tree containing a reparse point: $candidate"
        }
    }

    Remove-Item -LiteralPath $candidate -Recurse -Force
}

Write-Host "Development output cleanup completed."
