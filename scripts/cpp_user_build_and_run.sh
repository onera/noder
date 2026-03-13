#!/usr/bin/env bash

# usage:
# chmod +x cpp_user_build_and_run.sh
# ./cpp_user_build_and_run.sh

set -euo pipefail

PYTHON="${PYTHON:-python}"

py() {
  "$PYTHON" -c "$1"
}

command -v "$PYTHON" >/dev/null 2>&1 || {
  echo "Error: python not found"
  exit 1
}

command -v g++ >/dev/null 2>&1 || {
  echo "Error: g++ not found"
  exit 1
}

if ! "$PYTHON" -c "import noder, pybind11" >/dev/null 2>&1; then
  echo "Error: this Python environment must contain both noder and pybind11"
  echo "Install with: pip install noder pybind11"
  exit 1
fi

PY_VER="$("$PYTHON" -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')"
PY_BIN_DIR="$(dirname "$(command -v "$PYTHON")")"

PYTHON_CONFIG=""
for candidate in \
  "$PY_BIN_DIR/python${PY_VER}-config" \
  "$PY_BIN_DIR/python3-config" \
  "$(command -v python${PY_VER}-config 2>/dev/null || true)" \
  "$(command -v python3-config 2>/dev/null || true)" \
  "$(command -v python-config 2>/dev/null || true)"
do
  if [[ -n "$candidate" && -x "$candidate" ]]; then
    PYTHON_CONFIG="$candidate"
    break
  fi
done

if [[ -z "$PYTHON_CONFIG" ]]; then
  echo "Error: no python-config script was found"
  echo "Install the Python development package, for example:"
  echo "  Debian/Ubuntu: sudo apt install python3-dev"
  echo "  Fedora/RHEL:   sudo dnf install python3-devel"
  exit 1
fi

if ! "$PYTHON_CONFIG" --embed --ldflags >/dev/null 2>&1; then
  echo "Error: $PYTHON_CONFIG does not support --embed"
  echo "Install the Python development package for this interpreter"
  exit 1
fi

NODER_DIR="$(py 'import importlib.util, pathlib; spec = importlib.util.find_spec("noder"); print(pathlib.Path(list(spec.submodule_search_locations)[0]))')"
PYBIND11_INCLUDE="$(py 'import pybind11; print(pybind11.get_include())')"
PYTHON_LIBDIR="$(py 'import sysconfig; print(sysconfig.get_config_var("LIBDIR") or "")')"

CORE_SHARED_LIB="$(find "$NODER_DIR" -maxdepth 1 -type f \( -name 'libcore_shared.so' -o -name 'libcore_shared.so.*' \) | head -n 1)"

if [[ -z "$CORE_SHARED_LIB" ]]; then
  echo "Error: could not find libcore_shared.so in $NODER_DIR"
  exit 1
fi

PYTHON_INCLUDES=($("$PYTHON_CONFIG" --includes))
PYTHON_LDFLAGS=($("$PYTHON_CONFIG" --embed --ldflags))

RPATH_FLAGS=(-Wl,-rpath,"$NODER_DIR")
if [[ -n "$PYTHON_LIBDIR" ]]; then
  RPATH_FLAGS+=(-Wl,-rpath,"$PYTHON_LIBDIR")
fi

g++ \
  -std=c++20 \
  -O2 \
  -DENABLE_HDF5_IO \
  main.cpp \
  -I"$NODER_DIR/include" \
  -I"$PYBIND11_INCLUDE" \
  "${PYTHON_INCLUDES[@]}" \
  "$CORE_SHARED_LIB" \
  "${PYTHON_LDFLAGS[@]}" \
  "${RPATH_FLAGS[@]}" \
  -o main

export LD_LIBRARY_PATH="$NODER_DIR${PYTHON_LIBDIR:+:$PYTHON_LIBDIR}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

./main
ls -l hello_noder.cgns
