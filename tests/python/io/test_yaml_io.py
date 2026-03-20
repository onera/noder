from pathlib import Path

import numpy as np

from noder.core import Node
import noder.core.io as gio


def _new_cgns_tree():
    tree = Node("CGNSTree", "CGNSTree_t")
    version = Node("CGNSLibraryVersion", "CGNSLibraryVersion_t")
    version.set_data(np.array([4.0], dtype=np.float32))
    base = Node("Base", "CGNSBase_t")
    base.set_data(np.array([3, 3], dtype=np.int32))
    zone = Node("Zone", "Zone_t")
    zone.set_data(np.array([2, 1, 0], dtype=np.int32))
    base.add_child(zone)
    tree.add_children([version, base])
    return tree


def test_yaml_round_trip_python_tree(tmp_path):
    filename = str(tmp_path / "tree.yaml")

    root = Node("root", "UserDefinedData_t")
    matrix = Node("matrix", "DataArray_t")
    matrix_data = np.asfortranarray(np.arange(6, dtype=np.float64).reshape(2, 3))
    matrix.set_data(matrix_data)
    message = Node("message", "Descriptor_t")
    message.set_data("hello yaml")
    root.add_children([matrix, message])

    root.write(filename)
    read_root = gio.read(filename)

    restored_matrix = read_root.get_at_path("root/matrix").data().getPyArray()
    assert np.array_equal(restored_matrix, matrix_data)
    assert restored_matrix.flags["F_CONTIGUOUS"]
    assert read_root.get_at_path("root/message").data().extractString() == "hello yaml"


def test_yaml_round_trip_plain_inline_string(tmp_path):
    filename = Path(tmp_path) / "plain_string.yaml"

    root = Node("root", "UserDefinedData_t")
    message = Node("message", "Descriptor_t")
    message.set_data("hola")
    root.add_child(message)

    root.write(str(filename))
    read_root = gio.read(str(filename))

    assert "values: [hola]" in filename.read_text(encoding="utf-8")
    assert read_root.get_at_path("root/message").data().extractString() == "hola"


def test_yaml_round_trip_preserves_non_contiguous_array(tmp_path):
    filename = str(tmp_path / "view.yaml")

    root = Node("root")
    view_node = Node("view")
    view = np.arange(16, dtype=np.int32).reshape(4, 4)[:, ::2]
    assert not view.flags["C_CONTIGUOUS"]
    view_node.set_data(view)
    root.add_child(view_node)

    root.write(filename)
    read_root = gio.read(filename)

    restored = read_root.get_at_path("root/view").data().getPyArray()
    assert np.array_equal(restored, view)
    assert restored.strides == view.strides


def test_yaml_round_trip_links(tmp_path):
    filename = str(tmp_path / "links.yaml")

    root = Node("root")
    target = Node("target")
    target.set_data(np.array([1, 2, 3], dtype=np.int32))
    link = Node("target_link")
    link.set_link_target(".", "/root/target")
    root.add_children([target, link])

    root.write(filename)
    read_root = gio.read(filename)

    restored_link = read_root.get_at_path("root/target_link")
    assert restored_link is not None
    assert restored_link.has_link_target()
    assert restored_link.link_target_file() == "."
    assert restored_link.link_target_path() == "/root/target"


def test_yaml_reload_node_data(tmp_path):
    filename = str(tmp_path / "reload.yaml")

    root = Node("root")
    value = Node("value")
    root.add_child(value)
    value.set_data(np.array([3], dtype=np.int32))
    root.write(filename)

    value.set_data(np.array([9], dtype=np.int32))
    value.reload_node_data(filename)

    assert int(value.data().getPyArray()[0]) == 3


def test_yaml_save_this_node_only(tmp_path):
    filename = str(tmp_path / "save_one.yaml")

    root = Node("root")
    mutable_node = Node("mutable")
    stable_node = Node("stable")
    mutable_node.set_data(np.array([1], dtype=np.int32))
    stable_node.set_data(np.array([2], dtype=np.int32))
    root.add_children([mutable_node, stable_node])
    root.write(filename)

    mutable_node.set_data(np.array([99], dtype=np.int32))
    mutable_node.save_this_node_only(filename)

    read_root = gio.read(filename)
    assert int(read_root.get_at_path("root/mutable").data().getPyArray()[0]) == 99
    assert int(read_root.get_at_path("root/stable").data().getPyArray()[0]) == 2


def test_yaml_round_trip_preserves_cgns_tree_wrapper(tmp_path):
    filename = str(tmp_path / "tree.yaml")

    tree = _new_cgns_tree()
    tree.write(filename)

    read_tree = gio.read(filename)
    assert read_tree.name() == "CGNSTree"
    assert read_tree.type() == "CGNSTree_t"
    assert read_tree.get_at_path("CGNSTree/CGNSLibraryVersion") is not None
    assert read_tree.get_at_path("CGNSTree/Base/Zone") is not None


def test_yaml_output_is_readable(tmp_path):
    filename = Path(tmp_path) / "tree.yaml"

    root = Node("root", "UserDefinedData_t")
    child = Node("message", "Descriptor_t")
    child.set_data("hello yaml")
    root.add_child(child)
    root.write(str(filename))

    content = filename.read_text(encoding="utf-8")
    assert "format: noder-yaml" in content
    assert "root:" in content
    assert "children:" in content
    assert "name: message" in content
    assert "{" not in content
