# --- NODER dev helpers -------------------------------------------------------

function Set-NoderPythonPath {
  # Equivalent of: export PYTHONPATH=$(pwd)/dist/dev:$PYTHONPATH
  $devPath = (Join-Path (Get-Location) "dist\dev")
  if ([string]::IsNullOrWhiteSpace($env:PYTHONPATH)) {
    $env:PYTHONPATH = $devPath
  } else {
    # Windows uses ';' as the separator
    $env:PYTHONPATH = "$devPath;$env:PYTHONPATH"
  }
}

function Remove-NoderBuildArtifacts {
  # Equivalent of: rm -rf build dist
  foreach ($p in @("build", "dist")) {
    if (Test-Path $p) {
      Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $p
    }
  }
}

function Get-NoderLatestVersionDirectory {
  param([Parameter(Mandatory = $true)][string]$Path)

  if (-not (Test-Path -LiteralPath $Path)) {
    throw "Required tool directory was not found: $Path"
  }

  $latest = Get-ChildItem -LiteralPath $Path -Directory |
    Where-Object { $_.Name -match '^\d+(\.\d+)*$' } |
    Sort-Object { [version]$_.Name } -Descending |
    Select-Object -First 1
  if (-not $latest) {
    throw "No versioned tool directory was found under: $Path"
  }
  return $latest.FullName
}

function Add-NoderEnvironmentPaths {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string[]]$Paths
  )

  $current = [Environment]::GetEnvironmentVariable($Name, "Process")
  $existing = @($current -split ';' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
  $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
  foreach ($entry in $existing) { [void]$seen.Add($entry) }

  $combined = @()
  foreach ($entry in $Paths) {
    if ((Test-Path -LiteralPath $entry) -and $seen.Add($entry)) {
      $combined += $entry
    }
  }
  $combined += $existing
  [Environment]::SetEnvironmentVariable($Name, ($combined -join ';'), "Process")
}

function Initialize-NoderIntelBuildEnvironment {
  $programFilesX86 = [Environment]::GetEnvironmentVariable("ProgramFiles(x86)")
  $intelCompilerRoot = Join-Path $programFilesX86 "Intel\oneAPI\compiler\latest"
  $intelCompiler = Join-Path $intelCompilerRoot "bin\icx.exe"
  if (-not (Test-Path -LiteralPath $intelCompiler)) {
    throw "Intel oneAPI C++ compiler was not found: $intelCompiler"
  }

  $vswhere = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
  if (-not (Test-Path -LiteralPath $vswhere)) {
    throw "Visual Studio locator was not found: $vswhere"
  }
  $visualStudioRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1
  if ($visualStudioRoot) { $visualStudioRoot = $visualStudioRoot.Trim() }
  if (-not $visualStudioRoot) {
    throw "A Visual Studio installation with x64 C++ tools was not found."
  }

  $msvcRoot = Get-NoderLatestVersionDirectory (Join-Path $visualStudioRoot "VC\Tools\MSVC")
  $windowsKitRoot = Join-Path $programFilesX86 "Windows Kits\10"
  $windowsKitVersion = Get-NoderLatestVersionDirectory (Join-Path $windowsKitRoot "Lib") | Split-Path -Leaf

  $msvcBin = Join-Path $msvcRoot "bin\Hostx64\x64"
  $sdkBin = Join-Path $windowsKitRoot "bin\$windowsKitVersion\x64"
  $sdkLib = Join-Path $windowsKitRoot "Lib\$windowsKitVersion"
  $sdkInclude = Join-Path $windowsKitRoot "Include\$windowsKitVersion"
  $includePaths = @(
    (Join-Path $msvcRoot "include"),
    (Join-Path $sdkInclude "ucrt"),
    (Join-Path $sdkInclude "um"),
    (Join-Path $sdkInclude "shared"),
    (Join-Path $sdkInclude "winrt"),
    (Join-Path $sdkInclude "cppwinrt")
  )

  Add-NoderEnvironmentPaths "Path" @((Join-Path $intelCompilerRoot "bin"), $msvcBin, $sdkBin)
  Add-NoderEnvironmentPaths "Lib" @(
    (Join-Path $intelCompilerRoot "lib"),
    (Join-Path $msvcRoot "lib\x64"),
    (Join-Path $sdkLib "um\x64"),
    (Join-Path $sdkLib "ucrt\x64")
  )
  Add-NoderEnvironmentPaths "Include" $includePaths
  return $intelCompiler
}

function Invoke-NoderCMakeConfigure {
  param(
    [ValidateSet("INTEL", "MSVC")]
    [string]$Compiler = "INTEL"
  )

  if ($Compiler -eq "MSVC") {
    cmake -S . -B build/dev -G "Visual Studio 17 2022" -A x64
  } else {
    $intelCompiler = Initialize-NoderIntelBuildEnvironment
    cmake -S . -B build/dev -G Ninja "-DCMAKE_C_COMPILER=$intelCompiler" "-DCMAKE_CXX_COMPILER=$intelCompiler"
  }
  if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }
}

function Invoke-NoderCMakeBuild {
  param(
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release",
    [int]$Jobs = 8
  )
  cmake --build build/dev --config $Config --parallel $Jobs
  if ($LASTEXITCODE -ne 0) { throw "CMake build failed." }
}

function Invoke-NoderCMakeInstall {
  param(
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release"
  )
  cmake --install build/dev --config $Config --prefix dist/dev
  if ($LASTEXITCODE -ne 0) { throw "CMake install failed." }
}

function Invoke-NoderGenStubs {
  python scripts/gen_stubs.py
  if ($LASTEXITCODE -ne 0) { throw "Stub generation failed." }
}

function noder_init {
  Set-NoderPythonPath
  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure
  Invoke-NoderCMakeBuild -Config Release -Jobs 8
  Invoke-NoderCMakeInstall -Config Release
  Invoke-NoderGenStubs
}

function noder_build_install {
  Invoke-NoderCMakeBuild -Config Release -Jobs 8
  Invoke-NoderCMakeInstall -Config Release
  Invoke-NoderGenStubs
}

function noder_config_build_install {
  param(
    [ValidateSet("INTEL", "MSVC")]
    [string]$Compiler = "INTEL"
  )

  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure -Compiler $Compiler
  Invoke-NoderCMakeBuild -Config Release -Jobs 8
  Invoke-NoderCMakeInstall -Config Release
  Invoke-NoderGenStubs
}

function noder_config_build_install_seqwarn {
  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure

  # Equivalent of: cmake --build ... -j 1 2>output_warnings.txt
  # PowerShell redirects stderr with 2>
  cmake --build build/dev --config Release --parallel 1 2> output_warnings.txt
  if ($LASTEXITCODE -ne 0) { throw "CMake build failed (see output_warnings.txt)." }

  Invoke-NoderCMakeInstall -Config Release
  Invoke-NoderGenStubs
}

function noder_gen_stubs {
  Invoke-NoderGenStubs
}

# Optional: mimic your "export PYTHONPATH=/home/luis/noder/dist/dev"
# If you always want a specific fixed path regardless of current directory, uncomment and set it:
$env:PYTHONPATH = "C:\Users\aero_\AppData\Local\noder\dist\dev"
