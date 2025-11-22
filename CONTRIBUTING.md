Compiling from source
=====================

You can install using:

```
pip install . -v
```

and noder will be now part of your python packages.

Compiling during development process
====================================

Configuration step of pip may take long. If you want to speed-up this step,
you can avoid it by taking advantage of cmake caching capacity.

For this, you create a build directory called noder:
```
mkdir noder && cd noder
```

In that directory, create an empty __init__.py so that Python recognizes it as 
a package:
```
touch __init__.py
```

Then you may call cmake like this:
```
cmake ..
cmake --build .
```

For execution, do not forget to add your project path to the environment
variable PYTHONPATH:
```
cd ..
export PYTHONPATH=$(pwd):$PYTHONPATH
```

You may now launch the tests:
```
pytest
```

After slight modification of source code (in C++ for instance), you can 
recompile again from project root directory using:

```
cmake --build noder/ -j 4 # note "-j 4" for compiling using 4 threads in parallel
```

Hint:

A set of instructions for fast compilation during development process:

```
export PYTHONPATH=$(pwd)/dist/dev:$PYTHONPATH
rm -r build dist
cmake -B build/dev .
cmake --build build/dev -j 8
cmake --install build/dev --prefix dist/dev
```

Convenient alias for continuous modifications:
```
alias noder_build_install='cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev'
```

Convenient alias for entire process (new session):
```
alias noder_init='export PYTHONPATH=$(pwd)/dist/dev:$PYTHONPATH && rm -r build dist && cmake -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev'
```

Convenient alias for entire process (same session):
```
alias noder_config_build_install='rm -r build dist && cmake -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev'
```

For Windows using MINGW64:

```
export PYTHONPATH="$(cygpath -w "$PWD")/dist/dev;$PYTHONPATH"
rm -r build dist
cmake -G "Visual Studio 17 2022" -B build/dev .
cmake --build build/dev -j 8
cmake --install build/dev --prefix dist/dev --config Debug
```

And the associated aliases:

```
# Convert PWD to Windows format and set PYTHONPATH
alias noder_set_pythonpath='export PYTHONPATH="$(cygpath -w "$PWD")/dist/dev;$PYTHONPATH"'

# Build & Install without reconfiguring CMake (fast iteration)
alias noder_build_install='cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev --config Debug'

# Full Rebuild: Deletes build & reconfigures CMake
alias noder_config_build_install='rm -rf build dist && cmake -G "Visual Studio 17 2022" -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev --config Debug'
```

For Windows from a CMD terminal with access to x86 Native Tools Command Prompt for VS 20XX:
```
set PYTHONPATH=%CD%\dist\dev;%PYTHONPATH%
rmdir /s /q build dist 2>nul
cmake -G "Visual Studio 17 2022" -B build\dev .
cmake --build build\dev --config Release -j 8
cmake --install build\dev --prefix dist\dev --config Release
```

For Windows from a PowerShell (and access to Visual Studio compilers):
```
$env:PYTHONPATH = "$PWD\dist\dev;$env:PYTHONPATH"
Remove-Item -Recurse -Force build, dist -ErrorAction SilentlyContinue
cmake -G "Visual Studio 17 2022" -B build/dev .
cmake --build build/dev --config Release -j 8
cmake --install build/dev --prefix dist/dev --config Release
```


Example of Linux aliases:
```
alias noder_init='export PYTHONPATH=$(pwd)/dist/dev:$PYTHONPATH && rm -rf build dist && cmake -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev && python scripts/gen_stubs.py'
alias noder_build_install='cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev && python scripts/gen_stubs.py'
alias noder_config_build_install='rm -rf build dist && cmake -B build/dev . && cmake --build build/dev -j 8 && cmake --install build/dev --prefix dist/dev && python scripts/gen_stubs.py'
alias noder_config_build_install_seqwarn='rm -rf build dist && cmake -B build/dev . && cmake --build build/dev -j 1 2>output_warnings.txt && cmake --install build/dev --prefix dist/dev && python scripts/gen_stubs.py'
alias noder_gen_stubs='python scripts/gen_stubs.py'

export PYTHONPATH=/home/luis/noder/dist/dev
```