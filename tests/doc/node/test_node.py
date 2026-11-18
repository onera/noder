"""Executable documentation examples."""

import copy

import numpy as np

import pytest

from noder.core import Node, Array, pyCGNSToNode, nodeToPyCGNS

try:
    import noder.core.io as gio
    ENABLE_HDF5_IO = hasattr(gio, "write_numpy")
except ImportError:
    ENABLE_HDF5_IO = False

def get_all_paths_list(node, pathlist):
    pathlist += [ node.path() ]
    for child in node.children():
        get_all_paths_list(child, pathlist)

def get_all_paths(node):
    paths = []
    get_all_paths_list(node, paths)
    return paths

def _new_cgns_tree_with_base():
    tree = Node("CGNSTree", "CGNSTree_t")
    version = Node("CGNSLibraryVersion", "CGNSLibraryVersion_t")
    version.set_data(np.array([4.0], dtype=np.float32))
    base = Node("Base", "CGNSBase_t")
    base.set_data(np.array([3, 3], dtype=np.int32))
    tree.add_children([version, base])
    return tree, base

if __name__ == '__main__':
    test_print_tree()

pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target='noder.core.Node.add_child')
def test_add_child():
    # docs:start add_child
    from noder.core import Node

    # create nodes using default constructor
    a = Node("a")
    b = Node("b")

    a.add_child(b) # add b to a

    # check if we got the expected result
    expected_path_of_b = "a/b"
    assert b.path() == expected_path_of_b
    # docs:end add_child


@pytest.mark.doc_example(target='noder.core.Node.__init__')
def test_init():
    # docs:start init
    from noder.core import Node

    node = Node("zone", "Zone_t")
    assert node.name() == "zone"
    assert node.type() == "Zone_t"
    # docs:end init


@pytest.mark.doc_example(target='noder.core.Node.pick')
def test_pick():
    # docs:start pick
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    root.add_child(child)

    assert root.pick().by_name("child") is child
    # docs:end pick


@pytest.mark.doc_example(target='noder.core.Node.name')
def test_name():
    # docs:start name
    from noder.core import Node

    node = Node("zone")
    assert node.name() == "zone"
    # docs:end name


@pytest.mark.doc_example(target='noder.core.Node.set_name')
def test_set_name():
    # docs:start set_name
    from noder.core import Node

    node = Node("zone")
    node.set_name("zone_updated")
    assert node.name() == "zone_updated"
    # docs:end set_name


@pytest.mark.doc_example(target='noder.core.Node.type')
def test_type():
    # docs:start type
    from noder.core import Node

    node = Node("zone", "Zone_t")
    assert node.type() == "Zone_t"
    # docs:end type


@pytest.mark.doc_example(target='noder.core.Node.set_type')
def test_set_type():
    # docs:start set_type
    from noder.core import Node

    node = Node("zone")
    node.set_type("UserDefinedData_t")
    assert node.type() == "UserDefinedData_t"
    # docs:end set_type


@pytest.mark.doc_example(target='noder.core.Node.data')
def test_data():
    # docs:start data
    from noder.core import Node
    import numpy as np

    node = Node("values")
    node.set_data(np.array([1, 2, 3], dtype=np.int32))
    assert np.array_equal(node.data().getPyArray(), np.array([1, 2, 3], dtype=np.int32))
    # docs:end data


@pytest.mark.doc_example(target='noder.core.Node.set_data')
def test_set_data():
    # docs:start set_data
    from noder.core import Node
    import numpy as np

    node = Node("values")
    node.set_data([1.0, 2.0, 3.0])  # list/tuple are accepted and converted
    assert np.array_equal(node.data().getPyArray(), np.array([1.0, 2.0, 3.0]))
    # docs:end set_data


@pytest.mark.doc_example(target='noder.core.Node.children')
def test_children():
    # docs:start children
    from noder.core import Node

    root = Node("root")
    root.add_children([Node("a"), Node("b")])
    assert [child.name() for child in root.children()] == ["a", "b"]
    # docs:end children


@pytest.mark.doc_example(target='noder.core.Node.has_children')
def test_has_children():
    # docs:start has_children
    from noder.core import Node

    root = Node("root")
    assert not root.has_children()
    root.add_child(Node("child"))
    assert root.has_children()
    # docs:end has_children


@pytest.mark.doc_example(target='noder.core.Node.parent')
def test_parent():
    # docs:start parent
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    root.add_child(child)
    assert child.parent() is root
    # docs:end parent


@pytest.mark.doc_example(target='noder.core.Node.root')
def test_root():
    # docs:start root
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    leaf = Node("leaf")
    root.add_child(child)
    child.add_child(leaf)
    assert leaf.root() is root
    # docs:end root


@pytest.mark.doc_example(target='noder.core.Node.level')
def test_level():
    # docs:start level
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    leaf = Node("leaf")
    root.add_child(child)
    child.add_child(leaf)
    assert leaf.level() == 2
    # docs:end level


@pytest.mark.doc_example(target='noder.core.Node.position')
def test_position():
    # docs:start position
    from noder.core import Node

    root = Node("root")
    a = Node("a")
    b = Node("b")
    root.add_children([a, b])
    assert a.position() == 0
    assert b.position() == 1
    # docs:end position


@pytest.mark.doc_example(target='noder.core.Node.detach')
def test_detach():
    # docs:start detach
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    root.add_child(child)
    child.detach()
    assert child.parent() is None
    assert root.get_children_names() == []
    # docs:end detach


@pytest.mark.doc_example(target='noder.core.Node.attach_to')
def test_attach_to():
    # docs:start attach_to
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    child.attach_to(root)
    assert child.path() == "root/child"
    # docs:end attach_to


@pytest.mark.doc_example(target='noder.core.Node.add_children')
def test_add_children():
    # docs:start add_children
    from noder.core import Node

    root = Node("root")
    root.add_children([Node("a"), Node("b"), Node("c")])
    assert root.get_children_names() == ["a", "b", "c"]
    # docs:end add_children


@pytest.mark.doc_example(target='noder.core.Node.siblings')
def test_siblings():
    # docs:start siblings
    from noder.core import Node

    root = Node("root")
    a = Node("a")
    b = Node("b")
    c = Node("c")
    root.add_children([a, b, c])
    assert [n.name() for n in b.siblings(include_myself=False)] == ["a", "c"]
    # docs:end siblings


@pytest.mark.doc_example(target='noder.core.Node.has_siblings')
def test_has_siblings():
    # docs:start has_siblings
    from noder.core import Node

    root = Node("root")
    a = Node("a")
    b = Node("b")
    root.add_children([a, b])
    assert a.has_siblings()
    # docs:end has_siblings


@pytest.mark.doc_example(target='noder.core.Node.get_children_names')
def test_get_children_names():
    # docs:start get_children_names
    from noder.core import Node

    root = Node("root")
    root.add_children([Node("a"), Node("b")])
    assert root.get_children_names() == ["a", "b"]
    # docs:end get_children_names


@pytest.mark.doc_example(target='noder.core.Node.swap')
def test_swap():
    # docs:start swap
    from noder.core import Node

    left = Node("left")
    right = Node("right")
    b = Node("b")
    c = Node("c")
    left.add_child(b)
    right.add_child(c)
    b.swap(c)
    assert b.path() == "right/b"
    assert c.path() == "left/c"
    # docs:end swap


@pytest.mark.doc_example(target='noder.core.Node.copy')
def test_copy():
    # docs:start copy
    from noder.core import Node
    import numpy as np

    root = Node("root")
    root.set_data(np.array([1.0, 2.0]))
    shallow = root.copy()
    deep = root.copy(deep=True)
    assert np.shares_memory(root.data().getPyArray(), shallow.data().getPyArray())
    assert not np.shares_memory(root.data().getPyArray(), deep.data().getPyArray())
    # docs:end copy


@pytest.mark.doc_example(target='noder.core.Node.get_at_path')
def test_get_at_path():
    # docs:start get_at_path
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    root.add_child(child)
    assert root.get_at_path("root/child") is child
    # docs:end get_at_path


@pytest.mark.doc_example(target='noder.core.Node.path')
def test_path():
    # docs:start path
    from noder.core import Node

    root = Node("root")
    child = Node("child")
    root.add_child(child)
    assert child.path() == "root/child"
    # docs:end path


@pytest.mark.doc_example(target='noder.core.Node.descendants')
def test_descendants():
    # docs:start descendants
    from noder.core import Node

    root = Node("root")
    root.add_child(Node("a"))
    root.add_child(Node("b"))
    assert [n.name() for n in root.descendants()] == ["root", "a", "b"]
    # docs:end descendants


@pytest.mark.doc_example(target='noder.core.Node.merge')
def test_merge():
    # docs:start merge
    from noder.core import Node

    left = Node("Root")
    left.add_child(Node("A"))
    right = Node("Root")
    right.add_child(Node("B"))
    left.merge(right)
    assert left.get_children_names() == ["A", "B"]
    # docs:end merge


@pytest.mark.doc_example(target='noder.core.Node.has_link_target')
def test_has_link_target():
    # docs:start has_link_target
    from noder.core import Node

    node = Node("link")
    assert not node.has_link_target()
    node.set_link_target(".", "/root/target")
    assert node.has_link_target()
    # docs:end has_link_target


@pytest.mark.doc_example(target='noder.core.Node.link_target_file')
def test_link_target_file():
    # docs:start link_target_file
    from noder.core import Node

    node = Node("link")
    node.set_link_target("external.cgns", "/CGNSTree/Base")
    assert node.link_target_file() == "external.cgns"
    # docs:end link_target_file


@pytest.mark.doc_example(target='noder.core.Node.link_target_path')
def test_link_target_path():
    # docs:start link_target_path
    from noder.core import Node

    node = Node("link")
    node.set_link_target("external.cgns", "/CGNSTree/Base")
    assert node.link_target_path() == "/CGNSTree/Base"
    # docs:end link_target_path


@pytest.mark.doc_example(target='noder.core.Node.set_link_target')
def test_set_link_target():
    # docs:start set_link_target
    from noder.core import Node

    node = Node("link")
    node.set_link_target(".", "/root/target")
    assert node.has_link_target()
    # docs:end set_link_target


@pytest.mark.doc_example(target='noder.core.Node.clear_link_target')
def test_clear_link_target():
    # docs:start clear_link_target
    from noder.core import Node

    node = Node("link")
    node.set_link_target(".", "/root/target")
    node.clear_link_target()
    assert not node.has_link_target()
    # docs:end clear_link_target


@pytest.mark.doc_example(target='noder.core.Node.get_links')
def test_get_links():
    # docs:start get_links
    from noder.core import Node

    root = Node("root")
    link = Node("link")
    root.add_child(link)
    link.set_link_target(".", "/root/target")
    assert tuple(root.get_links()[0]) == (".", ".", "/root/link", "/root/target", 5)
    # docs:end get_links


@pytest.mark.doc_example(target='noder.core.Node.set_parameters')
def test_set_parameters():
    # docs:start set_parameters
    from noder.core import Node

    root = Node("root")
    root.set_parameters("Parameters", scalar=3.5, nested={"x": 1})
    params = root.get_parameters("Parameters", transform_numpy_scalars=True)
    assert params["scalar"] == 3.5
    assert params["nested"]["x"] == 1
    # docs:end set_parameters


@pytest.mark.doc_example(target='noder.core.Node.get_parameters')
def test_get_parameters():
    # docs:start get_parameters
    from noder.core import Node

    root = Node("root")
    root.set_parameters("Parameters", values=[1, 2, 3])
    params = root.get_parameters("Parameters")
    assert params["values"].shape == (3,)
    # docs:end get_parameters


@pytest.mark.doc_example(target='noder.core.Node.reload_node_data')
@pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")
def test_reload_node_data():
    # docs:start reload_node_data
    from noder.core import Node
    from pathlib import Path
    import tempfile
    import numpy as np

    with tempfile.TemporaryDirectory() as tmpdir:
        filename = str(Path(tmpdir) / "reload_node_data_example.cgns")
        root = Node("root")
        value = Node("value")
        root.add_child(value)
        value.set_data(np.array([3], dtype=np.int32))
        root.write(filename)
        value.set_data(np.array([9], dtype=np.int32))
        value.reload_node_data(filename)
        assert int(value.data().getPyArray()[0]) == 3
    # docs:end reload_node_data


@pytest.mark.doc_example(target='noder.core.Node.save_this_node_only')
@pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")
def test_save_this_node_only():
    # docs:start save_this_node_only
    from noder.core import Node
    from pathlib import Path
    import tempfile
    import numpy as np
    import noder.core.io as gio

    with tempfile.TemporaryDirectory() as tmpdir:
        filename = str(Path(tmpdir) / "save_this_node_only_example.cgns")
        root = Node("root")
        mutable_node = Node("mutable")
        mutable_node.set_data(np.array([1], dtype=np.int32))
        root.add_child(mutable_node)
        root.write(filename)
        mutable_node.set_data(np.array([99], dtype=np.int32))
        mutable_node.save_this_node_only(filename)
        read_root = gio.read(filename)
        assert int(read_root.get_at_path("root/mutable").data().getPyArray()[0]) == 99
    # docs:end save_this_node_only


@pytest.mark.doc_example(target='noder.core.Node.print_tree')
def test_print_tree():
    # docs:start print_tree
    from noder.core import Node

    root = Node("root")
    root.add_children([Node("a"), Node("b")])
    tree_text = root.print_tree()
    assert "root" in tree_text
    assert "a" in tree_text
    # docs:end print_tree


@pytest.mark.doc_example(target='noder.core.Node.write')
@pytest.mark.skipif(not ENABLE_HDF5_IO, reason="HDF5 support not enabled in the build.")
def test_write(tmp_path):
    # docs:start write
    from noder.core import Node
    import noder.core.io as gio

    filename = str(tmp_path / "write_example.cgns")
    root = Node("root")
    root.add_child(Node("child"))
    root.write(filename)
    read_root = gio.read(filename)
    assert read_root.get_at_path("root/child") is not None
    # docs:end write
