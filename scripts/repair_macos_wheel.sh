#!/bin/bash
set -euo pipefail

INPUT_WHEEL="$1"
OUTPUT_DIR="$2"
ARCHS="${CIBW_ARCHS_MACOS:-arm64}"
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-${CMAKE_OSX_DEPLOYMENT_TARGET:-15.0}}"
export MACOSX_DEPLOYMENT_TARGET

version_gt() {
    python - "$1" "$2" <<'PY'
import sys

def parts(value):
    return tuple(int(part) for part in value.split("."))

sys.exit(0 if parts(sys.argv[1]) > parts(sys.argv[2]) else 1)
PY
}

macos_min_version() {
    local binary="$1"
    local min_version

    min_version=$(otool -l "$binary" | awk '
        /cmd LC_BUILD_VERSION/ { in_build = 1; next }
        in_build && /minos/ { print $2; exit }
        /cmd LC_VERSION_MIN_MACOSX/ { in_legacy = 1; next }
        in_legacy && /version/ { print $2; exit }
    ')

    if [[ -n "$min_version" ]]; then
        echo "$min_version"
    fi
}

absolute_dylib_deps() {
    local binary="$1"
    otool -L "$binary" |
        awk 'NR > 1 { print $1 }' |
        grep -E '^/(opt/homebrew|usr/local|Library/Frameworks)/' || true
}

raise_deployment_target_for_binary_graph() {
    local root="$1"
    local queue=("$root")
    local seen=""

    while ((${#queue[@]})); do
        local binary="${queue[0]}"
        queue=("${queue[@]:1}")

        [[ -f "$binary" ]] || continue
        if [[ "$seen" == *"|$binary|"* ]]; then
            continue
        fi
        seen="$seen|$binary|"

        local min_version
        min_version=$(macos_min_version "$binary" || true)
        if [[ -n "$min_version" ]] && version_gt "$min_version" "$MACOSX_DEPLOYMENT_TARGET"; then
            echo "Raising MACOSX_DEPLOYMENT_TARGET from $MACOSX_DEPLOYMENT_TARGET to $min_version because $binary requires it"
            MACOSX_DEPLOYMENT_TARGET="$min_version"
            export MACOSX_DEPLOYMENT_TARGET
        fi

        while IFS= read -r dep; do
            [[ -f "$dep" ]] && queue+=("$dep")
        done < <(absolute_dylib_deps "$binary")
    done
}

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
echo "Initial MACOSX_DEPLOYMENT_TARGET: $MACOSX_DEPLOYMENT_TARGET"
raise_deployment_target_for_binary_graph "$DYLIB"
echo "Using MACOSX_DEPLOYMENT_TARGET: $MACOSX_DEPLOYMENT_TARGET"

echo "Copying $DYLIB into wheel's noder/ package"
cp "$DYLIB" "$WHEELDIR/noder/"

echo "Patching .so files to use @loader_path/libcore_shared.dylib"
for SOFILE in $(find "$WHEELDIR/noder" -name "*.so"); do
    echo "  -> Patching $SOFILE"
    install_name_tool -change "@rpath/libcore_shared.dylib" "@loader_path/libcore_shared.dylib" "$SOFILE"
done

# Repack into a new wheel
BASENAME=$(basename "$INPUT_WHEEL")
REPAIRED="$TMPDIR/$BASENAME"
echo "Repacking to $REPAIRED"
(cd "$WHEELDIR" && zip -qr "$REPAIRED" .)

echo "Running delocate on $REPAIRED -> $OUTPUT_DIR"
delocate-wheel --require-archs "$ARCHS" -w "$OUTPUT_DIR" -v "$REPAIRED"

echo "Done repairing macOS wheel."
