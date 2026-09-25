"""Smoke tests for the optional cgnsviz terminal executable."""

import os
from pathlib import Path
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

        child = h5file.create_group("SmokeData", track_order=True)
        child.attrs["name"] = np.bytes_("SmokeData")
        child.attrs["label"] = np.bytes_("DataArray_t")
        child.attrs["type"] = np.bytes_("I4")
        child.create_dataset(" data", data=np.array([7, 11], dtype=np.int32))

        large_child = h5file.create_group("LargeData", track_order=True)
        large_child.attrs["name"] = np.bytes_("LargeData")
        large_child.attrs["label"] = np.bytes_("DataArray_t")
        large_child.attrs["type"] = np.bytes_("I4")
        large_child.create_dataset(" data", data=np.arange(65, dtype=np.int32))


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
        [str(executable), str(filename), "--non-interactive"],
        input="\nj\nk\nj\n\nk\nj\nq\n",
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
    assert result.stdout.count("Array int32 [ 7 11 ]") >= 2
    assert result.stdout.count("[too big size to show]") >= 2
