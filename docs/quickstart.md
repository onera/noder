# User quick start

Noder is a Python package. Install a supported Python version first if you do
not already have one: [download Python from python.org](https://www.python.org/downloads/).
Noder requires Python 3.8 or newer.

## Install Noder

Open a terminal in your project and install the current PyPI release:

```console
python -m pip install noder
```

Using `python -m pip` makes sure pip installs into the Python interpreter named
by `python`. On Windows, if your interpreter is launched with `py`, you can use
`py -m pip` instead.

For isolated projects, create and activate a virtual environment before
installing:

```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
python -m pip install noder
```

On Linux or macOS, activate it with:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install noder
```

## Upgrade Noder

Run this in the same environment where Noder is installed:

```console
python -m pip install --upgrade noder
```

## Install a specific release

Pin the exact release you need with `==`. This example selects Noder 1.5.10;
replace it with another published version when needed:

```console
python -m pip install "noder==VERSION"
```

Replace `VERSION` with the release number you want, for example a version
listed on [Noder's PyPI release history](https://pypi.org/project/noder/#history).
Check the installed version from Python:

```python
import noder

print(noder.__version__)
```

For supported Python and platform combinations, pip can install a prebuilt
wheel. If it falls back to building from source, the build also needs a C++20
compiler and HDF5 development files. See the
[Contributing guide](contributing.md) for source-build requirements.
