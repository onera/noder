import os, shutil
import pytest
import numpy as np
import noder.array.data_types as dtypes

try:
    import h5py
    HAS_H5PY = True
except ImportError:
    HAS_H5PY = False

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

    b = node.pick().by_name("b")
    assert b is not None
    assert len(b.data().getPyArray()) == 5

    c = node.pick().by_name("c")
    assert c is not None
    assert c.data().extractString() == "toto"

@pytest.mark.skipif(not HAS_H5PY, reason="h5py not available")
def test_write_nodes_cgns_attrs_layout(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test_attrs.hdf5')

    from noder.core import new_node
    a = new_node("a", "DataArray_t")
    b = new_node("b", "DataArray_t")
    c = new_node("c", "DataArray_t")
    c.set_data(np.array([1, 2, 3], dtype=np.int32))
    a / b / c
    a.write(tmp_filename)

    with h5py.File(tmp_filename, "r") as f:
        def _as_str(v):
            if isinstance(v, bytes):
                return v.decode("utf-8").rstrip("\x00")
            return str(v).rstrip("\x00")

        a = f["/a"]
        c = f["/a/b/c"]

        assert set(a.attrs.keys()) == {"name", "label", "type", "flags"}
        assert _as_str(a.attrs["name"]) == "a"
        assert _as_str(a.attrs["label"]) == "DataArray_t"
        assert _as_str(a.attrs["type"]) == "MT"
        assert int(np.asarray(a.attrs["flags"])[0]) == 1

        assert _as_str(c.attrs["type"]) == "I4"
        assert "type" not in f["/a/b/c/ data"].attrs

        # CGNS/HDF5 readers such as cgnsview expect tracked+indexed link order.
        assert f["/"].id.get_create_plist().get_link_creation_order() == 3
        assert a.id.get_create_plist().get_link_creation_order() == 3
        assert f["/a/b"].id.get_create_plist().get_link_creation_order() == 3
        assert c.id.get_create_plist().get_link_creation_order() == 3

def test_write_link_nodes(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test_links.hdf5')
    giocpp.test_write_link_nodes(tmp_filename)

def test_read_links(tmp_path):
    os.makedirs(tmp_path, exist_ok=True)
    tmp_filename = str(tmp_path/'test_links.hdf5')

    node = giocpp.test_read_links(tmp_filename)
    link_node = node.pick().by_name("target_link")

    assert link_node is not None
    assert link_node.has_link_target()
    assert link_node.link_target_file() == "."
    assert link_node.link_target_path() == "/root/target"

if __name__ == '__main__':
    # test_write_and_read_numerical_numpy(np.int8, "F")
    from timeit import default_timer as toc
    tic=toc()
    # a = gio.read("fields.cgns")
    tic= toc()-tic
    # print(a)
    print(f"elapsed time: {tic} s")
    # test_read()
