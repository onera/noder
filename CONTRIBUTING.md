# Contributing

General
-------

Developments must be compatible for compilation and execution in multiple platforms, including Linux (Ubuntu, Rocky...), Windows and macOS.

The code must be able build and deploy using command:
```
pip install .
```

The objective is having a code able to use GitHub's Actions CI/CD capacities in order to produce pre-compiled binaries for all aforementiond platforms and multiple python versions and upload them towards PyPi. Final user will install the code using the single command `pip install noder`.

Requirements
------------

Building from source requires:

- CPython 3.8 or newer, including the development headers and library for the same Python interpreter that CMake will use. Install Python from [python.org](https://www.python.org/downloads/).
- [CMake 3.15 or newer](https://cmake.org/download/).
- A C++20-capable compiler: GCC or Clang on Linux, Apple Clang on macOS, or MSVC on Windows. Intel oneAPI's DPC++/C++ compiler is also an option on supported Ubuntu and Windows versions; on Windows it requires the MSVC C++ build tools.
- The HDF5 C library and development headers. The default build enables HDF5 I/O and `cgnsvi`, so HDF5 is required. Use an HDF5 build compatible with the selected compiler and architecture; see [The HDF Group's downloads](https://www.hdfgroup.org/download-hdf5/).

`pip install .` installs the Python build requirements declared in `pyproject.toml`. It does not install a native C++ compiler or HDF5. Installing a prebuilt `noder` wheel from PyPI does not require these source-build tools.

Platform setup:

- **Linux (Debian/Ubuntu):** install the build tools and development packages with:

  ```bash
  sudo apt install build-essential cmake pkg-config libhdf5-dev python3-dev
  ```

  For Fedora/RHEL/Rocky Linux, the corresponding packages are typically installed with:

  ```bash
  sudo dnf install gcc-c++ cmake pkgconf-pkg-config hdf5-devel python3-devel
  ```

  If using a non-system CPython, install or build its matching development headers instead of `python3-dev`/`python3-devel`.

  **Intel oneAPI option:** follow Intel's [oneAPI Linux installation guide](https://www.intel.com/content/www/us/en/docs/oneapi-toolkit/installation-guide-linux/latest/overview.html) and install the toolkit package (`intel-oneapi-toolkit`) from Intel's APT repository. In each build shell, initialize its environment and configure a fresh build directory with the Intel compiler drivers:

  ```bash
  source /opt/intel/oneapi/setvars.sh
  cmake -S . -B build/oneapi -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx
  ```

- **macOS:** install Apple's [Xcode Command Line Tools](https://developer.apple.com/documentation/xcode/installing-the-command-line-tools), then install CMake, HDF5, and pkg-config with [Homebrew](https://brew.sh/):

  ```bash
  xcode-select --install
  brew install cmake hdf5 pkg-config
  ```

- **Windows:** install [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/) with the *Desktop development with C++* workload and a Windows SDK. Install CPython and CMake as well. The repository's [HDF5 setup script](scripts/setup_hdf5_windows.ps1) downloads the HDF5 build for Visual Studio 2022 and prints the paths CMake needs. Run it from a scratch directory (it writes `hdf5.zip` to the current directory), then set the printed HDF5 paths in the build environment.

  **Intel oneAPI option:** install the [Intel oneAPI Toolkit](https://www.intel.com/content/www/us/en/docs/oneapi-toolkit/installation-guide-windows/latest/overview.html) after the Visual Studio C++ workload. Open an Intel oneAPI command prompt configured for Visual Studio 2022, or initialize the shell with the toolkit's `setvars.bat`. The repository's [PowerShell build profile](scripts/Microsoft.PowerShell_profile.ps1) also supports `noder_config_build_install -Compiler INTEL`.

If HDF5 is installed outside the platform's usual search paths, set `HDF5_DIR` or `CMAKE_PREFIX_PATH` so CMake can find its package configuration. On Windows, the project also accepts `HDF5_C_LIBRARY` and `HDF5_INCLUDE_DIR`. If HDF5 is unavailable, configure with `-DENABLE_CGNSVI=OFF`; HDF5/CGNS I/O and `cgnsvi` will not be available.

To run the Python test suite, install its optional dependencies with `python -m pip install ".[test]"`.

# Conventions

C++ source code
---------------

Implementation details shall be done in C++, not in Python.

Every atomic function and class in C++ shall have its Python-binded counterpart.

Source code in C++ shall be placed inside `src/c++`, including headers and binding code of `pybind11`.

Classes shall use `PascalCase`.
Methods, functions, and variables shall use `camelCase`.
No `snake_case` in C++ code.

Implementation files must end with `.cpp`.
Headers must end with `.hpp`.
Both file and directory names must be written using `snake_case`.

Testing code of C++ shall be placed inside `tests/c++`, including headers and binding code of `pybind11`. C++ code is tested using `pytest`. The Python entry points of C++ testable code shall have the form `test_*_cpp.py`, indicating that the test code actually tests direct C++ code (hence tests shall be binded as well).

Python source code
------------------

Source code in Python shall be located in `src/python/noder`.

Classes shall be written using `PascalCase`.
Methods, functions, and variables shall use `snake_case`.
No `camelCase` usage in Python code except on direct wraps towards C++ code or for migration code (which is supposed to be cleanly removed at some point).

Python files must end with `.py`.
Both file and directory names must be written using `snake_case`.

Testing code of Python shall be placed inside `tests/python`.


Coding practices
----------------

The developer is invited to make its best to respect the very good writing advices presented by Robert C. Martin in its book "Clean Code".

The most important aspect of all is READABILITY, and this must be the ultimate goal.

Hence, we highlight some of the most relevant aspects for this codebase:

1. Use explicit, meaningful names for everything: classes, methods, modules, functions, namespaces, variables, file names, folder names. Very short names shall only be used in functions with small scope, or for highly conventional elements.

2. Write small classes, short functions, short methods, short files. This may require adding extra layers of abstraction, splitting classes, and using multiple files, but this is a desirable feature: different layers provides meaning. Avoid mixing highly applicative concepts (e.g. physical modeling) with very low-level features (strides computation, row vs column major conventions optimizations...).

3. Use always only one single class per file. The class name shall match the file name where it is contained. Classes must have one single obvious responsability. If a class has more than one responsability, then split it in different classes.

4. Avoid writing useless comments. Rather make your best to make your source code "read" naturally.

5. Each class, method, function shall have its corresponding test. Such test may follow the same structure as the source code, and will be located in `tests`. The objective is having small functions/classes/method, with small explicit tests. Test code shall follow the same style and coding guidelines than the original source code. We aim total coverage by tests. Unit tests shall be very fast to execute.

6. Minimize the number of arguments of a function. Always prefer classes+methods rather than functions.

7. Prefer data encapsulation (using classes) rather than exposing public data (using modules and functions).

8. Conveniently include exceptions handling. Segmentation faults shall be avoided and shall be immediately corrected. The code shall return clear, explicative error messages. When an exception involve data stored in a `noder` object, always provide the path of the problematic data: this is extremely helpful for bug identification.

9. Try to use the "journalistic style": In a file, put first the most meaninful methods or functions. Then, progressively to the bottom the "details". This is not always possible in C++, but is highly feasible in Python. 

10. Prefer polymorphism over set of instructions `if/else` or `switch/case`. Make use of abstract classes for allowing this.

# Architecture

General guidelines
------------------

By architecture, we mean both the folder & file structure of the code and the dependency flow. 
Developer shall make its best to follow the excellent advices of Robert C. Martin in its book "Clean Architecture".

We point out some relevant architecture choices of this codebase, a.k.a "good pratices":

1. When making new files, always prefer a deep level approach instead of a flat approach. In other words, use deep folder structure because different folders levels provides context. This is a useful rule of thumb when organizing files after a Class refactoring required to verify the Single Responsability Principle that led to multiple classes. You may group them simply into a folder.

2. When coding, always try to make your code open for extensions, but closed for modifications. This means that including new functionalities should not imply modifying large parts of existing code, but rather writing new code with minimal or no modification of existing one.

3. Try to keep a clear dependency flow. Low-level features shall depend on the code highest level rules, and not the other way round.

# Daily coding workflow

The source code can be built and installed using the very simple command `pip install .` This shall be done at least once (for example, after cloning from `git`), because it will properly install all required dependencies, including dependencies for developpers, indicated in file `pyproject.toml`.

However, this command may take some time and is not well suited for everyday coding, where multiple compiling calls may be done. Or very small Python code modifications want to be tested on-the-fly.


Working in both C++ and Python layers
-------------------------------------

Instead of running `pip install .`, for everyday work you'd better use specific compilation commands based in `cmake`. The proposed workflow consists in creating and using development folders in `build/dev` and `dist/dev`.

Hence, you will need to add `dist/dev` to your PYTHONPATH environment variable:

In bash:
```bash
export PYTHONPATH={noder_project_root}/dist/dev:$PYTHONPATH
```

In PowerShell:

```powershell
$env:PYTHONPATH = "{noder_project_root}\dist\dev;$env:PYTHONPATH"
```

Since you may want to have access to `cgnsvi`, you will also need to update your PATH avriable:

In bash:
```bash
export PATH={noder_project_root}/dist/dev/noder:$PATH
```

In PowerShell:

```powershell
$env:PATH = "{noder_project_root}\dist\dev\noder;$env:PATH"
```


Then, two classical commands are proposed depending on the kind of operations being involved:

* `noder_config_build_install`: entire process of configuration+compile(build)+install into `dist/dev` for usage. Use this if you create new or delete existing source files, or if you modified `CMakeLists.txt`. It shall be done at least once. Then, if development only involves modification of existing C++ files, prefer the next command which is fastest:
* `noder_build_install`: compile(build)+install into `dist/dev`. Reuses configuration step, hence this is the fastest approach.

If you are working in Linux, you may use these convenient aliases to your `~/.bashrc` or profile (do not forget to adapt the number of processes to your machine, in this example we use 8):

```bash
alias noder_config_build_install='rm -rf build dist && cmake -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev'
alias noder_build_install='cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev'
```

If you are working in Windows, you may add these convenient commands to your PowerShell profile (`$PROFILE`) (again, adapt the number of processes to your machine, in this snippet of code we use 8):

```powershell
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
  param(
    [ValidateSet("INTEL", "MSVC")]
    [string]$Compiler = "INTEL"
  )

  if ($Compiler -eq "MSVC") {
    cmake -S . -B build/dev -G "Visual Studio 17 2022" -A x64
  } else {
    Initialize-IntelBuildEnvironment
    $intelCompiler = 'C:/Program Files (x86)/Intel/oneAPI/compiler/latest/bin/icx.exe'
    cmake -S . -B build/dev -G Ninja -DCMAKE_BUILD_TYPE=Release `
      "-DCMAKE_C_COMPILER=$intelCompiler" `
      "-DCMAKE_CXX_COMPILER=$intelCompiler"
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

# CMake install generates stubs from the extension in its install prefix.
function Invoke-NoderCMakeInstall {
  param(
    [ValidateSet("Debug","Release","RelWithDebInfo","MinSizeRel")]
    [string]$Config = "Release"
  )
  cmake --install build/dev --config $Config --prefix dist/dev
  if ($LASTEXITCODE -ne 0) { throw "CMake install failed." }
}

function Invoke-NoderGenStubs {
  python scripts/gen_stubs.py --import-root dist/dev
  if ($LASTEXITCODE -ne 0) { throw "Stub generation failed." }
}

function noder_init {
  Initialize-IntelBuildEnvironment
  Set-NoderPythonPath
  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure
  Invoke-NoderCMakeBuild -Config Release -Jobs 8
  Invoke-NoderCMakeInstall -Config Release
}

function noder_build_install {
  Initialize-IntelBuildEnvironment
  Invoke-NoderCMakeBuild -Config Release -Jobs 8
  Invoke-NoderCMakeInstall -Config Release
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
}

function noder_config_build_install_seqwarn {
  Initialize-IntelBuildEnvironment
  Remove-NoderBuildArtifacts
  Invoke-NoderCMakeConfigure

  # Equivalent of: cmake --build ... -j 1 2>output_warnings.txt
  # PowerShell redirects stderr with 2>
  cmake --build build/dev --config Release --parallel 1 2> output_warnings.txt
  if ($LASTEXITCODE -ne 0) { throw "CMake build failed (see output_warnings.txt)." }

  Invoke-NoderCMakeInstall -Config Release
}

function noder_gen_stubs {
  Invoke-NoderGenStubs
}
```

# Additional considerations

- *BEWARE*: NEVER use `pip install . -e`, since this usually creates conflicts for this codebase.

