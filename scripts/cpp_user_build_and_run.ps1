# usage:
# Set-ExecutionPolicy -Scope Process Bypass
# .\cpp_user_build_and_run.ps1

$ErrorActionPreference = 'Stop'

function Py([string]$code) {
    (& python -c $code).Trim()
}

$NoderDir = Py "import importlib.util, pathlib; spec=importlib.util.find_spec('noder'); print(pathlib.Path(list(spec.submodule_search_locations)[0]))"
$Pybind11Include = Py "import pybind11; print(pybind11.get_include())"
$PythonInclude = Py "import sysconfig; print(sysconfig.get_path('include'))"
$PythonBase = Py "import sys; print(sys.base_prefix)"
$PythonLib = Py "import sys; print(f'python{sys.version_info.major}{sys.version_info.minor}.lib')"
$PythonLibDir = Join-Path $PythonBase 'libs'

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
if (-not $vsPath) { throw 'Visual Studio Build Tools with C++ support were not found.' }

$vsDevCmd = Join-Path $vsPath 'Common7\Tools\VsDevCmd.bat'
cmd /c "`"$vsDevCmd`" -arch=x64 && set" | ForEach-Object {
    if ($_ -match '^(.*?)=(.*)$') {
        Set-Item -Path "Env:$($matches[1])" -Value $matches[2]
    }
}

& cl.exe `
  /nologo /std:c++17 /EHsc /MD /DENABLE_HDF5_IO `
  /I "$NoderDir\include" `
  /I "$PythonInclude" `
  /I "$Pybind11Include" `
  main.cpp `
  /link `
  "/LIBPATH:$NoderDir" core_shared.lib `
  "/LIBPATH:$PythonLibDir" $PythonLib `
  /OUT:main.exe

if ($LASTEXITCODE -ne 0) { throw "Compilation failed with exit code $LASTEXITCODE" }

$env:PATH = "$NoderDir;$PythonBase;$env:PATH"

& .\main.exe
if ($LASTEXITCODE -ne 0) { throw "Execution failed with exit code $LASTEXITCODE" }

Get-Item .\hello_noder.cgns

