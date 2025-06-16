#!/bin/bash
set -euo pipefail

INPUT_WHEEL="$1"
OUTPUT_DIR="$2"
ARCHS="${CIBW_ARCHS_MACOS:-arm64}"
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

# Extract Python version tag (e.g., cp38, cp310)
PYVER=$(basename "$INPUT_WHEEL" | grep -oE 'cp3[0-9]{1,2}')

if [[ -z "$PYVER" ]]; then
    echo "Could not parse Python version from wheel filename: $INPUT_WHEEL"
    exit 1
fi

WHEELDIR="$TMPDIR/wheel"
echo "Unzipping $INPUT_WHEEL to $WHEELDIR"
unzip -q "$INPUT_WHEEL" -d "$WHEELDIR"

echo "Searching for libcore_shared.dylib under build/* for Python tag: $PYVER"
DYLIB=$(find build -type f -path "*/noder/libcore_shared.dylib" | grep "$PYVER" | head -n 1)

if [[ -z "$DYLIB" || ! -f "$DYLIB" ]]; then
    echo "Could not find libcore_shared.dylib for $PYVER in build/*"
    echo "Available .dylib files:"
    find build -name "libcore_shared.dylib" || true
    exit 1
fi

echo "Found dylib: $DYLIB"
echo "Copying $DYLIB into wheel's noder/ package"
cp "$DYLIB" "$WHEELDIR/noder/"

echo "Patching .so files to use @loader_path/libcore_shared.dylib"
for SOFILE in $(find "$WHEELDIR/noder" -name "*.so"); do
    echo "  ➔ Patching $SOFILE"
    install_name_tool -change "@rpath/libcore_shared.dylib" "@loader_path/libcore_shared.dylib" "$SOFILE"
done

# Repack into a new wheel
BASENAME=$(basename "$INPUT_WHEEL")
REPAIRED="$TMPDIR/$BASENAME"
echo "Repacking to $REPAIRED"
(cd "$WHEELDIR" && zip -qr "$REPAIRED" .)

echo "Running delocate on $REPAIRED → $OUTPUT_DIR"
delocate-wheel --require-archs "$ARCHS" -w "$OUTPUT_DIR" -v "$REPAIRED"

echo "Done repairing macOS wheel."
