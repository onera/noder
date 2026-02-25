#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_DIR="$(dirname -- "$SCRIPT_DIR")"

DOXYGEN_BIN=""
if command -v doxygen >/dev/null 2>&1; then
  DOXYGEN_BIN="doxygen"
elif [ -x "/c/Program Files/doxygen/bin/doxygen.exe" ]; then
  DOXYGEN_BIN="/c/Program Files/doxygen/bin/doxygen.exe"
elif [ -x "/mnt/c/Program Files/doxygen/bin/doxygen.exe" ]; then
  DOXYGEN_BIN="/mnt/c/Program Files/doxygen/bin/doxygen.exe"
fi

if [ -z "$DOXYGEN_BIN" ]; then
  echo "doxygen binary not found. Install doxygen and retry." >&2
  exit 1
fi

rm -rf "$PROJECT_DIR/docs/_doxygen/xml" "$PROJECT_DIR/docs/api/cpp/generated"
"$DOXYGEN_BIN" "$PROJECT_DIR/docs/Doxyfile"
python -m sphinx -E -W -b html "$PROJECT_DIR/docs" "$PROJECT_DIR/docs/html"
