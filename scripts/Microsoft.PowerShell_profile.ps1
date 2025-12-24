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

function Invoke-NoderCMakeConfigure {
  # Equivalent of: cmake -B build/dev .
  cmake -B build/dev .
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
  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure
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
