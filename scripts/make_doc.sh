#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_DIR="$(dirname -- "$SCRIPT_DIR")"

python3 -m sphinx -E -W -b html "$PROJECT_DIR/docs" "$PROJECT_DIR/docs/html"
