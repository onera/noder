# Execution:
# Set-ExecutionPolicy -Scope Process Bypass
# .\build-and-run.ps1

$ErrorActionPreference = 'Stop'

$NoderDir = if ($env:NODER_DIR) {
    $env:NODER_DIR
} else {
    'C:\Users\aero_\AppData\Local\noder\dist\dev\noder'
}

if (-not (Test-Path $NoderDir)) {
    throw "noder package directory not found: $NoderDir"
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found at $vswhere"
}

$vsPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
if (-not $vsPath) {
    throw 'Visual Studio Build Tools with C++ support were not found.'
}

$vsDevCmd = Join-Path $vsPath 'Common7\Tools\VsDevCmd.bat'
cmd /c "`"$vsDevCmd`" -arch=x64 && set" | ForEach-Object {
    if ($_ -match '^(.*?)=(.*)$') {
        Set-Item -Path "Env:$($matches[1])" -Value $matches[2]
    }
}

Remove-Item .\main.obj, .\main.exe, .\hello_noder.cgns -Force -ErrorAction SilentlyContinue

& cl.exe `
  /nologo /std:c++17 /EHsc /MD /DENABLE_HDF5_IO `
  /I "$NoderDir\include" `
  main.cpp `
  /link `
  "/LIBPATH:$NoderDir" core_shared.lib `
  /OUT:main.exe

if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed with exit code $LASTEXITCODE"
}

$env:PATH = "$NoderDir;$env:PATH"

& .\main.exe
if ($LASTEXITCODE -ne 0) {
    throw "Execution failed with exit code $LASTEXITCODE"
}

Get-Item .\hello_noder.cgns
