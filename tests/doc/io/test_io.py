"""Executable examples for HDF5 and CGNS input/output."""

from pathlib import Path
from tempfile import TemporaryDirectory

import numpy as np
import pytest

import noder
import noder.core.io as io


pytestmark = pytest.mark.doc
ENABLE_HDF5_IO = hasattr(io, "write_numpy")


@pytest.mark.doc_example(target="noder.core.io.read")
@pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")
def test_read_cgns_file():
    # docs:start read_cgns_file
    x, y = np.meshgrid(np.arange(3.0), np.arange(2.0), indexing="ij")
    zone = noder.new_zone_from_arrays("Wing", ["x", "y"], [x, y])
    tree = noder.new_tree(AeroBase=[zone])
    noder.new_node(
        "CGNSLibraryVersion",
        "CGNSLibraryVersion_t",
        np.array([4.0], dtype=np.float32),
        parent=tree,
    )

    with TemporaryDirectory() as temporary_directory:
        filename = Path(temporary_directory) / "wing.cgns"
        tree.write(str(filename))
        restored = io.read(str(filename))

    assert restored.get_at_path("AeroBase/Wing") is not None
    # docs:end read_cgns_file


@pytest.mark.doc_example(target="noder.core.io.write_numpy")
@pytest.mark.doc_example(target="noder.core.io.read_numpy")
@pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")
def test_numpy_hdf5_round_trip():
    # docs:start numpy_hdf5_round_trip
    values = np.arange(6, dtype=np.float64).reshape(2, 3)
    with TemporaryDirectory() as temporary_directory:
        filename = Path(temporary_directory) / "values.h5"
        io.write_numpy(values, str(filename))
        restored = io.read_numpy(str(filename))
    np.testing.assert_array_equal(restored, values)
    # docs:end numpy_hdf5_round_trip
