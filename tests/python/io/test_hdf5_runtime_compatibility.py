import subprocess
import sys

import pytest


@pytest.mark.skipif(sys.platform != "win32", reason="Windows DLL isolation regression test")
@pytest.mark.parametrize(
    "imports",
    [
        "import noder; import h5py",
        "import h5py; import noder",
    ],
)
def test_noder_and_h5py_can_share_a_process(imports):
    exercise_both_libraries = """
import tempfile
from pathlib import Path

import numpy as np

with tempfile.TemporaryDirectory() as directory:
    directory = Path(directory)
    h5py_filename = directory / "h5py.h5"
    with h5py.File(h5py_filename, "w") as h5file:
        h5file["values"] = np.array([1, 2, 3], dtype=np.int32)
    with h5py.File(h5py_filename, "r") as h5file:
        np.testing.assert_array_equal(h5file["values"][()], [1, 2, 3])

    noder_filename = directory / "noder.h5"
    noder.write_numpy(np.array([4, 5, 6], dtype=np.int32), str(noder_filename))
    np.testing.assert_array_equal(noder.read_numpy(str(noder_filename)), [4, 5, 6])
"""
    subprocess.run(
        [sys.executable, "-c", f"{imports}\n{exercise_both_libraries}"],
        check=True,
    )
