"""Smoke tests for the optional cgnsviz terminal executable."""

import os
from pathlib import Path
import re
import shutil
import subprocess
from typing import Optional

import numpy as np
import pytest


def _cgnsviz_executable() -> Optional[Path]:
    configured = os.environ.get("CGNSVIZ_EXECUTABLE")
    candidates = []
    if configured:
        candidates.append(Path(configured))

    on_path = shutil.which("cgnsviz")
    if on_path:
        candidates.append(Path(on_path))

    repository_root = Path(__file__).resolve().parents[3]
    candidates.append(repository_root / "dist" / "dev" / "noder" / "cgnsviz.exe")
    candidates.append(repository_root / "dist" / "dev" / "noder" / "cgnsviz")
    candidates.append(repository_root / "build" / "dev" / "Release" / "cgnsviz.exe")
    candidates.append(repository_root / "build" / "dev" / "Release" / "cgnsviz")

    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def _write_smoke_file(filename: Path) -> None:
    h5py = pytest.importorskip("h5py")
    with h5py.File(filename, "w", track_order=True) as h5file:
        h5file.attrs["name"] = np.bytes_("HDF5 MotherNode")
        h5file.attrs["label"] = np.bytes_("Root Node of HDF5 File")
        h5file.attrs["type"] = np.bytes_("MT")

        zone_type = h5file.create_group("ZoneType", track_order=True)
        zone_type.attrs["name"] = np.bytes_("ZoneType")
        zone_type.attrs["label"] = np.bytes_("ZoneType_t")
        zone_type.attrs["type"] = np.bytes_("C1")
        zone_type.create_dataset(
            " data", data=np.frombuffer(b"Structured", dtype=np.int8)
        )

        small_numbers = h5file.create_group("SmallNumbers", track_order=True)
        small_numbers.attrs["name"] = np.bytes_("SmallNumbers")
        small_numbers.attrs["label"] = np.bytes_("DataArray_t")
        small_numbers.attrs["type"] = np.bytes_("I4")
        small_numbers.create_dataset(" data", data=np.arange(9, dtype=np.int32))

        child = h5file.create_group("SmokeData", track_order=True)
        child.attrs["name"] = np.bytes_("SmokeData")
        child.attrs["label"] = np.bytes_("DataArray_t")
        child.attrs["type"] = np.bytes_("I4")
        child.create_dataset(" data", data=np.arange(30, dtype=np.int32))

        large_child = h5file.create_group("LargeData", track_order=True)
        large_child.attrs["name"] = np.bytes_("LargeData")
        large_child.attrs["label"] = np.bytes_("DataArray_t")
        large_child.attrs["type"] = np.bytes_("I4")
        large_child.create_dataset(" data", data=np.arange(75, dtype=np.int32))

        for index in range(30):
            filler = h5file.create_group(f"Child{index:03d}", track_order=True)
            filler.attrs["name"] = np.bytes_(f"Child{index:03d}")
            filler.attrs["label"] = np.bytes_("UserDefinedData_t")
            filler.attrs["type"] = np.bytes_("MT")

        long_text = "FirstWord " + ("middle " * 18) + "LastWord"
        long_string = h5file.create_group("LongText", track_order=True)
        long_string.attrs["name"] = np.bytes_("LongText")
        long_string.attrs["label"] = np.bytes_("Descriptor_t")
        long_string.attrs["type"] = np.bytes_("C1")
        long_string.create_dataset(
            " data",
            data=np.frombuffer(long_text.encode("ascii"), dtype=np.int8),
        )


def test_cgnsviz_non_interactive_smoke(tmp_path):
    executable = _cgnsviz_executable()
    expected = os.environ.get("NODER_EXPECT_CGNSVIZ") == "1"
    expected = expected or os.environ.get("CI", "").lower() == "true"
    if executable is None and expected:
        pytest.fail("cgnsviz executable was expected but is not available")
    if executable is None:
        pytest.skip("cgnsviz executable is not available; configure with ENABLE_CGNSVIZ=ON")
    assert executable is not None
    filename = tmp_path / "cgnsviz-smoke.cgns"
    _write_smoke_file(filename)

    environment = os.environ.copy()
    runtime_directories = [str(executable.parent)]
    package_directory = executable.parents[1] / "noder"
    if package_directory.is_dir():
        runtime_directories.append(str(package_directory))
    build_runtime_directory = executable.parents[1] / "cp312" / "noder" / "Release"
    if build_runtime_directory.is_dir():
        runtime_directories.append(str(build_runtime_directory))
    environment["PATH"] = os.pathsep.join(runtime_directories + [environment.get("PATH", "")])

    result = subprocess.run(
        [str(executable), str(filename), "--non-interactive", "--max-chars", "20"],
        input=(
            "\n"
            "j\n"
            "j\x1b[27;2;13~\x1b[6~\n"
            "j\n"
            "\x1b[F\n"
            "\x1b[27;2;13~\x1b[6~\n"
            "\x1b[Hq\n"
        ),
        text=True,
        capture_output=True,
        check=False,
        env=environment,
    )

    assert result.returncode == 0, result.stderr
    assert "cgnsviz" in result.stdout
    assert "SmokeData" in result.stdout
    assert "DataArray_t" in result.stdout
    assert "[press Enter to show payload]" in result.stdout
    assert "cgnsviz  payload view" in result.stdout
    assert "[Enter] back" in result.stdout
    plain_output = re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", result.stdout)
    assert "ZoneType  ZoneType_t  Structured" in plain_output
    assert "SmallNumbers  DataArray_t  Array int32 [ 0 1 2 3 4 5 6 7 8 ]" in plain_output
    assert "payload (9 element(s), int32): Array int32 min=0, max=8, mean=4, median=4" in plain_output
    assert "payload (9 element(s), int32): Array int32 [ 0 1 2 3 4 5 6 7 8 ]" not in plain_output
    assert "[ 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 ]" in plain_output
    assert "[too big size to show]" not in plain_output
    assert "[min=0, max=74, mean=37, median=37]" in plain_output
    assert '[big str: 20 words "FirstWord ... LastWord"]' in plain_output
    assert "previous children hidden" in plain_output
    assert "FirstWord" in plain_output
    assert "LastWord" in plain_output
    assert not any(
        "SmokeData  DataArray_t  Array int32" in line
        for line in plain_output.splitlines()
    )
