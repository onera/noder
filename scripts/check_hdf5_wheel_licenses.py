"""Fail a release build if its wheels omit HDF5 redistribution notices."""

from __future__ import annotations

import re
import sys
from pathlib import Path
from zipfile import BadZipFile, ZipFile


REQUIRED_LICENSES = {
    "HDF5-COPYING.txt": (
        "Copyright Notice and License Terms",
        "Redistribution and use in source and binary forms",
    ),
    "HDF5-LICENSE.txt": (
        "Copyright Notice and License Terms",
        "Redistribution and use in source and binary forms",
    ),
    "zlib-LICENSE.txt": (
        "Jean-loup Gailly",
        "Permission is granted to anyone to use this software",
    ),
    "libaec-LICENSE.txt": (
        "Copyright 2024 Mathis Rosenhauer",
        "Redistribution and use in source and binary forms",
    ),
    "OpenSSL-1.1.1-LICENSE.txt": (
        "OpenSSL License",
        "original SSLeay license",
    ),
    "OpenSSL-3-LICENSE.txt": (
        "Apache License",
        "Version 2.0",
    ),
    "THIRD_PARTY_NOTICES.md": (
        "HDF5",
        "libaec",
        "license texts",
    ),
    "HDF5-BUILD-INFO.txt": (
        "HDF5 version:",
        "HDF5 linkage:",
        "HDF5 license file:",
    ),
}


def package_license_member(names: set[str], filename: str) -> str | None:
    suffix = f"/noder/licenses/{filename}"
    candidates = [
        name
        for name in names
        if name == f"noder/licenses/{filename}" or name.endswith(suffix)
    ]
    return candidates[0] if len(candidates) == 1 else None


def check_wheel(wheel_path: Path) -> list[str]:
    errors = []
    try:
        archive = ZipFile(wheel_path)
    except BadZipFile:
        return [f"{wheel_path.name}: not a valid ZIP wheel"]

    with archive:
        names = set(archive.namelist())
        contents = {}
        for filename, expected_text in REQUIRED_LICENSES.items():
            member = package_license_member(names, filename)
            if member is None:
                errors.append(
                    f"{wheel_path.name}: missing or duplicated "
                    f"noder/licenses/{filename}"
                )
                continue
            text = archive.read(member).decode("utf-8", errors="replace")
            contents[filename] = text
            for phrase in expected_text:
                if phrase not in text:
                    errors.append(
                        f"{wheel_path.name}: {filename} is missing expected "
                        f"text {phrase!r}"
                    )

    build_info = contents.get("HDF5-BUILD-INFO.txt", "")
    version_match = re.search(
        r"^HDF5 version: (\d+)\.\d+\.\d+", build_info, re.MULTILINE
    )
    if version_match is None:
        errors.append(f"{wheel_path.name}: HDF5 version was not recorded in build info")
    else:
        expected_hdf5_license = (
            "HDF5-COPYING.txt"
            if version_match.group(1) == "1"
            else "HDF5-LICENSE.txt"
        )
        if f"HDF5 license file: {expected_hdf5_license}" not in build_info:
            errors.append(
                f"{wheel_path.name}: build info selects the wrong HDF5 license file"
            )

    linkage_match = re.search(r"^HDF5 linkage: (.+)$", build_info, re.MULTILINE)
    if linkage_match is None:
        errors.append(f"{wheel_path.name}: HDF5 linkage was not recorded in build info")
    elif linkage_match.group(1).startswith("shared"):
        has_hdf5_runtime = any(
            "hdf5" in filename
            and (
                filename.endswith(".dll")
                or filename.endswith(".dylib")
                or ".so" in filename
            )
            for name in names
            for filename in [name.rsplit("/", 1)[-1].lower()]
        )
        if not has_hdf5_runtime:
            errors.append(
                f"{wheel_path.name}: shared HDF5 runtime is missing from the wheel"
            )
    elif not linkage_match.group(1).startswith("static"):
        errors.append(f"{wheel_path.name}: unrecognized HDF5 linkage in build info")

    return errors


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} WHEELHOUSE", file=sys.stderr)
        return 2

    wheelhouse = Path(sys.argv[1])
    wheels = sorted(wheelhouse.glob("*.whl"))
    if not wheels:
        print(f"No wheels found in {wheelhouse}", file=sys.stderr)
        return 1

    errors = []
    for wheel in wheels:
        errors.extend(check_wheel(wheel))

    if errors:
        print("HDF5 redistribution checks failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print(f"Verified HDF5 redistribution notices in {len(wheels)} wheel(s):")
    for wheel in wheels:
        with ZipFile(wheel) as archive:
            member = package_license_member(
                set(archive.namelist()), "HDF5-BUILD-INFO.txt"
            )
            assert member is not None
            build_info = archive.read(member).decode("utf-8", errors="replace").strip()
        version = next(
            line for line in build_info.splitlines() if line.startswith("HDF5 version:")
        )
        print(f"- {wheel.name}: {version.partition(':')[2].strip()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
