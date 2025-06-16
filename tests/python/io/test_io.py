import os, shutil
import pytest
import numpy as np
import noder.array.data_types as dtypes

try:
    import noder.core.io as gio
    import noder.tests.io as giocpp
    ENABLE_HDF5_IO = True
except ImportError:
    ENABLE_HDF5_IO = False

pytestmark = pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")

@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
@pytest.mark.parametrize("order", ["C", "F"])
def test_write_and_read_numerical_numpy(tmp_path, dtype, order):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')
    a = np.array([[1,2,3],[4,5,6]],dtype=dtypes.to_numpy_dtype[dtype], order=order)
    gio.write_numpy(a, tmp_filename)
    b = gio.read_numpy(tmp_filename, order=order)
    assert np.all(a==b)
    assert a.flags['C_CONTIGUOUS'] == b.flags['C_CONTIGUOUS']
    assert a.flags['F_CONTIGUOUS'] == b.flags['F_CONTIGUOUS']

@pytest.mark.parametrize("input", ["tortilla", b"tortilla", list(b"tortilla")])
def test_write_and_read_str(tmp_path, input):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')
    a = np.array(input)
    gio.write_numpy(a, tmp_filename)
    b = gio.read_numpy(tmp_filename)

    a_str = ''.join(str(a.tobytes().decode('utf-8')).split('\x00'))
    b_str = ''.join(str(b.tobytes().decode('utf-8')).split('\x00'))

    assert a_str == b_str

def test_write_str_byte(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')
    
    test_string = b"hello world!"
    gio.write_numpy(np.array(test_string), tmp_filename)
    
    b = gio.read_numpy(tmp_filename)
    a_str = ''.join(str(test_string.decode('utf-8')).split('\x00'))
    b_str = ''.join(str(b.tobytes().decode('utf-8')).split('\x00'))

    assert a_str == b_str

def test_write_str_byte_list(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')
    
    test_string = b"hello world!"
    gio.write_numpy(np.array([test_string]), tmp_filename)
    
    b = gio.read_numpy(tmp_filename)
    a_str = ''.join(str(test_string.decode('utf-8')).split('\x00'))
    b_str = ''.join(str(b.tobytes().decode('utf-8')).split('\x00'))

    assert a_str == b_str

def test_write_nodes(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')

    giocpp.test_write_nodes(tmp_filename)

def test_read(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test.hdf5')

    node = giocpp.test_read(tmp_filename)

    b = node.nav().byName("b")
    assert b is not None
    assert len(b.data().getPyArray()) == 5

    c = node.nav().byName("c")
    assert c is not None
    assert c.data().extractString() == "toto"

if __name__ == '__main__':
    # test_write_and_read_numerical_numpy(np.int8, "F")
    from timeit import default_timer as toc
    tic=toc()
    # a = gio.read("fields.cgns")
    tic= toc()-tic
    # print(a)
    print(f"elapsed time: {tic} s")
    # test_read()
