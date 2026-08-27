Write-Host "Generating engine, Parkour, application, and test projects..."

function Find-PremakeExecutable {
    $localPremakePath = Join-Path -Path $PSScriptRoot -ChildPath "premake5.exe"
    if (Test-Path -LiteralPath $localPremakePath -PathType Leaf) {
        return (Resolve-Path -LiteralPath $localPremakePath).Path
    }

    $premakeFromPath = Get-Command -Name "premake5.exe" -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($premakeFromPath) {
        return $premakeFromPath.Path
    }

    # A process does not see PATH changes made after it was started. Check the
    # persisted values too, and tolerate the common mistake of adding the exe
    # itself instead of its containing directory.
    $persistedPath = @(
        [Environment]::GetEnvironmentVariable("Path", "User"),
        [Environment]::GetEnvironmentVariable("Path", "Machine")
    ) -join ";"

    foreach ($entry in ($persistedPath -split ";")) {
        $candidate = [Environment]::ExpandEnvironmentVariables($entry.Trim().Trim('"'))
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }
        if ([System.IO.Path]::GetFileName($candidate) -ieq "premake5.exe") {
            if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                return (Resolve-Path -LiteralPath $candidate).Path
            }
        } else {
            $candidate = Join-Path -Path $candidate -ChildPath "premake5.exe"
            if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                return (Resolve-Path -LiteralPath $candidate).Path
            }
        }
    }

    return $null
}

$premakeCommand = Find-PremakeExecutable
if (-not $premakeCommand) {
    Write-Error "premake5.exe was not found. Add the directory containing premake5.exe to PATH, then restart PowerShell, or place premake5.exe in the repository root."
    exit 1
}

Write-Host "Using premake5.exe: $premakeCommand"

try {
    Push-Location -LiteralPath $PSScriptRoot
    & $premakeCommand vs2026

    if ($LASTEXITCODE -eq 0) {
        Write-Host "Premake5 execution completed."
    } else {
        throw "Premake5 exited with code $LASTEXITCODE."
    }
} catch {
    Write-Error "Failed to generate projects: $_"
    exit 1
} finally {
    Pop-Location
}
