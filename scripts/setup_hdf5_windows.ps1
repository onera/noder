$ErrorActionPreference = "Stop"

# Download and unzip outer archive
$url = "https://github.com/HDFGroup/hdf5/releases/download/hdf5_1.14.6/hdf5-1.14.6-win-vs2022_cl.zip"
Write-Host "Downloading: $url"
Invoke-WebRequest -Uri $url -OutFile hdf5.zip -Verbose

$outerExtractDir = "$env:USERPROFILE\hdf5"
Write-Host "Extracting outer ZIP to $outerExtractDir"
Expand-Archive -Path hdf5.zip -DestinationPath $outerExtractDir -Force

# Find the inner ZIP file recursively
$innerZip = Get-ChildItem -Path $outerExtractDir -Recurse -Filter "*.zip" |
  Where-Object { $_.Name -like "*win64.zip" } |
  Select-Object -First 1

if (-not $innerZip) {
  Write-Error "Could not find inner HDF5 zip inside: $outerExtractDir"
  Get-ChildItem -Recurse $outerExtractDir
  exit 1
}

$innerZipPath = $innerZip.FullName
$installRoot = Join-Path $outerExtractDir "install"
Write-Host "Extracting inner ZIP: $innerZipPath to $installRoot"
Expand-Archive -Path $innerZipPath -DestinationPath $installRoot -Force

# Locate the HDF5 directory
$hdf5Dir = Get-ChildItem -Path $installRoot -Directory |
  Where-Object { $_.Name -like "HDF5-*" } |
  Select-Object -First 1

if (-not $hdf5Dir) {
  Write-Error "HDF5 directory not found inside: $installRoot"
  Get-ChildItem -Recurse $installRoot
  exit 1
}
$hdf5Path = $hdf5Dir.FullName

# Required files
$libPath = Join-Path $hdf5Path "lib\hdf5.lib"
$includePath = Join-Path $hdf5Path "include\hdf5.h"
$binPath = Join-Path $hdf5Path "bin"

# Validation
Write-Host "Checking for $libPath"
if (!(Test-Path $libPath)) {
  Write-Error "HDF5 library not found: $libPath"
  exit 1
}

Write-Host "Checking for $includePath"
if (!(Test-Path $includePath)) {
  Write-Error "HDF5 header not found: $includePath"
  exit 1
}

# Print useful output
Write-Host "`nHDF5 successfully extracted and verified"
Write-Host "`nEnvironment settings (for local testing):"
Write-Host "  HDF5_ROOT:        $hdf5Path"
Write-Host "  HDF5_C_LIBRARY:   $libPath"
Write-Host "  HDF5_INCLUDE_DIR: $($hdf5Path)\include"
Write-Host "  bin PATH:         $binPath"

# Export for GitHub Actions if running in CI
if ($env:GITHUB_ENV) {
  echo "HDF5_ROOT=$hdf5Path" >> $env:GITHUB_ENV
  echo "HDF5_C_LIBRARY=$libPath" >> $env:GITHUB_ENV
  echo "HDF5_INCLUDE_DIR=$($hdf5Path)\include" >> $env:GITHUB_ENV
}

if ($env:GITHUB_PATH) {
  echo "$binPath" | Out-File -FilePath $env:GITHUB_PATH -Append
} else {
  Write-Host "GITHUB_PATH not set: skipping PATH update (expected when run locally)"
}
