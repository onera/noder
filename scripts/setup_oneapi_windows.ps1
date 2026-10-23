$ErrorActionPreference = "Stop"

# Keep this version aligned with Intel's documented Windows installer and CI sample.
$oneApiVersion = "2026.1.0.191"
$installerUrl = "https://registrationcenter-download.intel.com/akdlm/IRC_NAS/4144bec3-82ce-4672-bd71-5c93a79cd5e7/intel-oneapi-toolkit-${oneApiVersion}_offline.exe"
$installerPath = Join-Path $env:RUNNER_TEMP "intel-oneapi-toolkit-$oneApiVersion-offline.exe"
$component = "intel.oneapi.win.cpp-dpcpp-common"
$installRoot = "C:\Program Files (x86)\Intel\oneAPI"
$compilerPath = Join-Path $installRoot "compiler\latest\bin\icx.exe"
$logDirectory = Join-Path $env:RUNNER_TEMP "oneapi-installer-logs"
$bootstrapperLog = Join-Path $logDirectory "bootstrapper.log"

function Write-OneApiInstallerLogs {
    $logFiles = @(
        Get-ChildItem -LiteralPath $logDirectory -File -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension -in @(".log", ".txt") } |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 3
    )

    if ($logFiles.Count -eq 0) {
        Write-Host "No Intel oneAPI installer log files were found in $logDirectory"
        return
    }

    foreach ($logFile in $logFiles) {
        Write-Host "----- Intel oneAPI installer log: $($logFile.FullName) -----"
        Get-Content -LiteralPath $logFile.FullName -Tail 100
    }
}

Write-Host "Downloading Intel oneAPI C++ compiler $oneApiVersion"
Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath

New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null

Write-Host "Installing Intel oneAPI C++ compiler component: $component"
$installerArguments = @(
    "-l", $bootstrapperLog,
    "-s",
    "-a",
    "--silent",
    "--eula", "accept",
    "--components=$component",
    "-p=NEED_VS2022_INTEGRATION=0",
    "--log-dir=$logDirectory"
)

try {
    $installerProcess = Start-Process `
        -FilePath $installerPath `
        -ArgumentList $installerArguments `
        -Wait `
        -PassThru
} catch {
    Write-Host "Could not start the Intel oneAPI installer: $_"
    Write-OneApiInstallerLogs
    throw
}

$installerExitCode = $installerProcess.ExitCode
if ($null -eq $installerExitCode) {
    Write-OneApiInstallerLogs
    throw "Intel oneAPI installer exited without reporting a process exit code."
}

if ($installerExitCode -ne 0 -and $installerExitCode -ne 3010) {
    Write-OneApiInstallerLogs
    throw "Intel oneAPI installer failed with exit code $installerExitCode"
}

if (-not (Test-Path $compilerPath)) {
    Write-OneApiInstallerLogs
    throw "Intel oneAPI compiler was not found at $compilerPath"
}

Remove-Item -LiteralPath $installerPath -Force
Write-Host "Intel oneAPI compiler installed at $compilerPath"
