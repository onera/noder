$ErrorActionPreference = "Stop"

# Keep this version aligned with Intel's oneAPI CI sample configuration.
$oneApiVersion = "2026.2.0.661"
$installerUrl = "https://registrationcenter-download.intel.com/akdlm/IRC_NAS/728c4418-1b40-47a9-ad51-31530fb27c94/intel-oneapi-toolkit-${oneApiVersion}_offline.exe"
$installerPath = Join-Path $env:RUNNER_TEMP "intel-oneapi-toolkit-$oneApiVersion-offline.exe"
$component = "intel.oneapi.win.cpp-dpcpp-common"
$installRoot = "C:\Program Files (x86)\Intel\oneAPI"
$compilerPath = Join-Path $installRoot "compiler\latest\bin\icx.exe"

Write-Host "Downloading Intel oneAPI C++ compiler $oneApiVersion"
Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath

Write-Host "Installing Intel oneAPI C++ compiler component: $component"
& $installerPath -s -a --silent --eula accept "--components=$component" "-p=NEED_VS2022_INTEGRATION=0" "--log-dir=$env:RUNNER_TEMP"
if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne 3010) {
    throw "Intel oneAPI installer failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path $compilerPath)) {
    throw "Intel oneAPI compiler was not found at $compilerPath"
}

Remove-Item -LiteralPath $installerPath -Force
Write-Host "Intel oneAPI compiler installed at $compilerPath"
