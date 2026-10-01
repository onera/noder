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
        nested = zone_type.create_group("NestedSmoke", track_order=True)
        nested.attrs["name"] = np.bytes_("NestedSmoke")
        nested.attrs["label"] = np.bytes_("UserDefinedData_t")
        nested.attrs["type"] = np.bytes_("MT")
        nested_leaf = nested.create_group("NestedLeaf", track_order=True)
        nested_leaf.attrs["name"] = np.bytes_("NestedLeaf")
        nested_leaf.attrs["label"] = np.bytes_("UserDefinedData_t")
        nested_leaf.attrs["type"] = np.bytes_("C1")
        nested_leaf.create_dataset(
            " data", data=np.frombuffer(b"leaf-one", dtype=np.int8)
        )
        nested_leaf_two = nested.create_group("NestedLeafTwo", track_order=True)
        nested_leaf_two.attrs["name"] = np.bytes_("NestedLeafTwo")
        nested_leaf_two.attrs["label"] = np.bytes_("UserDefinedData_t")
        nested_leaf_two.attrs["type"] = np.bytes_("C1")
        nested_leaf_two.create_dataset(
            " data", data=np.frombuffer(b"leaf-two", dtype=np.int8)
        )
        matrix = nested.create_group("MatrixData", track_order=True)
        matrix.attrs["name"] = np.bytes_("MatrixData")
        matrix.attrs["label"] = np.bytes_("DataArray_t")
        matrix.attrs["type"] = np.bytes_("I4")
        matrix.create_dataset(" data", data=np.arange(6, dtype=np.int32).reshape(2, 3))

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


def _write_root_search_file(filename: Path) -> None:
    h5py = pytest.importorskip("h5py")
    with h5py.File(filename, "w", track_order=True) as h5file:
        h5file.attrs["name"] = np.bytes_("HDF5 MotherNode")
        h5file.attrs["label"] = np.bytes_("Root Node of HDF5 File")
        h5file.attrs["type"] = np.bytes_("MT")

        for base_name, zone_name in (("BaseOne", "ZoneOne"), ("BaseTwo", "ZoneTwo")):
            base = h5file.create_group(base_name, track_order=True)
            base.attrs["name"] = np.bytes_(base_name)
            base.attrs["label"] = np.bytes_("CGNSBase_t")
            base.attrs["type"] = np.bytes_("MT")
            zone = base.create_group(zone_name, track_order=True)
            zone.attrs["name"] = np.bytes_(zone_name)
            zone.attrs["label"] = np.bytes_("Zone_t")
            zone.attrs["type"] = np.bytes_("MT")


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
            "d\n"
            "j\n"
            "d\n"
            "j\n"
            "D\n"
            "j\x1b[6~\n"
            "\x1b"
            "j\n"
            "d\n"
            "\x1b[F\n"
            "d\n"
            "\x1bq\n"
        ),
        text=True,
        capture_output=True,
        check=False,
        env=environment,
    )

    assert result.returncode == 0, result.stderr
    assert "cgnsviz" in result.stdout
    assert re.search(
        r"CGNSviz from package NODER v[0-9]+\.[0-9]+\.[0-9]+ \(c\) ONERA",
        result.stdout,
    )
    assert "SmokeData" in result.stdout
    assert "DataArray_t" in result.stdout
    assert "[press d to show payload]" in result.stdout
    assert "cgnsviz  payload view" in result.stdout
    assert "[Escape] back" in result.stdout
    plain_output = re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", result.stdout)
    assert "ZoneType  ZoneType_t  Structured" in plain_output
    assert "SmallNumbers  DataArray_t  Array int32 [ 0 1 2 3 4 5 6 7 8 ]" in plain_output
    assert "payload (9 element(s), int32, shape=9): min=0, max=8, mean=4, median=4" in plain_output
    assert "payload (9 element(s), int32, shape=9): Array int32" not in plain_output
    assert "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29" in plain_output
    assert "[ 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 ]" not in plain_output
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


def test_cgnsviz_predicate_search_smoke(tmp_path):
    executable = _cgnsviz_executable()
    if executable is None:
        pytest.skip("cgnsviz executable is not available; configure with ENABLE_CGNSVIZ=ON")

    filename = tmp_path / "cgnsviz-search.cgns"
    _write_smoke_file(filename)
    environment = os.environ.copy()
    environment["PATH"] = os.pathsep.join(
        [str(executable.parent), environment.get("PATH", "")]
    )

    result = subprocess.run(
        [str(executable), str(filename), "--non-interactive"],
        input=(
            "l\n"
            "/n:NestedLeaf*\n"
            "j\n"
            "\n"
            "\x1b[27;2;13~\n"
            "m\n"
            "l\n"
            "h\n"
            "\\n:NestedSmoke\n"
            "h\n"
            "/q:invalid\n"
            "m\n"
            "h\n"
            "/\n"
            "\x1bq\n"
            "q\n"
        ),
        text=True,
        capture_output=True,
        check=False,
        env=environment,
    )

    assert result.returncode == 0, result.stderr
    assert "cgnsviz  matches view" in result.stdout
    assert "search: /n:NestedLeaf*    matches: 2" in result.stdout
    assert "Search /n:NestedLeaf*: 2 matches." in result.stdout
    assert "NestedLeaf" in result.stdout
    assert "leaf-two" in result.stdout
    assert "cgnsviz  payload view" in result.stdout
    assert "[Escape] back to node view" in result.stdout
    assert "[m] matches" in result.stdout
    assert "[Escape] node view" in result.stdout
    assert "Match 'NestedLeafTwo' has no children." in result.stdout
    assert "Search \\n:NestedSmoke: 1 match." in result.stdout
    assert "Search error: invalid predicate" in result.stdout
    assert "search: \\n:NestedSmoke    matches: 1" in result.stdout
    assert "search: /    matches: 0" in result.stdout
    assert "(no matches)" in result.stdout
    assert "[n/N]" not in result.stdout

    plain_output = re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", result.stdout)
    assert "NestedLeafTwo  UserDefinedData_t  ZoneType/NestedSmoke" in plain_output
    last_screen = result.stdout.rsplit("\x1b[2J\x1b[H", maxsplit=1)[-1]
    assert "cgnsviz  matches view" not in last_screen
    assert "path:" in last_screen


def test_cgnsviz_multidimensional_payload_metadata(tmp_path):
    executable = _cgnsviz_executable()
    if executable is None:
        pytest.skip("cgnsviz executable is not available; configure with ENABLE_CGNSVIZ=ON")

    filename = tmp_path / "cgnsviz-multidimensional.cgns"
    _write_smoke_file(filename)
    environment = os.environ.copy()
    environment["PATH"] = os.pathsep.join(
        [str(executable.parent), environment.get("PATH", "")]
    )

    result = subprocess.run(
        [str(executable), str(filename), "--non-interactive"],
        input=(
            "l\n"
            "l\n"
            "\x1b[F\n"
            "d\n"
            "D\n"
            "\x1bq\n"
            "q\n"
        ),
        text=True,
        capture_output=True,
        check=False,
        env=environment,
    )

    assert result.returncode == 0, result.stderr
    plain_output = re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", result.stdout)
    assert "MatrixData  DataArray_t  Array int32 [ 0 1 2 3 4 5 ]" in plain_output
    assert "payload (6 element(s), int32, shape=3x2): min=0, max=5, mean=2.5, median=2.5" in plain_output
    assert "elements: 6    shape: 3x2" in plain_output
    payload_screens = [
        re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", screen)
        for screen in result.stdout.split("\x1b[2J\x1b[H")
        if "cgnsviz  payload view" in screen
    ]
    assert payload_screens
    assert "0 1 2 3 4 5" in payload_screens[-1]
    assert "[ 0 1 2 3 4 5 ]" not in payload_screens[-1]


def test_cgnsviz_ctrl_home_searches_from_root(tmp_path):
    executable = _cgnsviz_executable()
    if executable is None:
        pytest.skip("cgnsviz executable is not available; configure with ENABLE_CGNSVIZ=ON")

    filename = tmp_path / "cgnsviz-root-search.cgns"
    _write_root_search_file(filename)
    environment = os.environ.copy()
    environment["PATH"] = os.pathsep.join(
        [str(executable.parent), environment.get("PATH", "")]
    )

    result = subprocess.run(
        [str(executable), str(filename), "--non-interactive"],
        input=(
            "/t:Zone_t\n"
            "\x1b[1;5H"
            "/t:Zone_t\n"
            "q\n"
        ),
        text=True,
        capture_output=True,
        check=False,
        env=environment,
    )

    assert result.returncode == 0, result.stderr
    assert "Search /t:Zone_t: 1 match." in result.stdout
    assert "Search /t:Zone_t: 2 matches." in result.stdout
    assert "ZoneOne" in result.stdout
    assert "ZoneTwo" in result.stdout
    assert "selection: root" in result.stdout
