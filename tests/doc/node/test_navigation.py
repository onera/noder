"""Executable documentation examples."""

import numpy as np

import pytest

from noder.core import Node, Array

def _build_navigation_example_tree():
    root = Node("root", "CGNSTree_t")

    zone_a = Node("Zone_A", "Zone_t")
    zone_a.set_data("prefix_A")
    zone_b = Node("Zone_B", "Zone_t")
    zone_b.set_data("prefix_B")
    root.add_children([zone_a, zone_b])

    coords_a = Node("Coords", "DataArray_t")
    coords_a.set_data(3)
    marker = Node("Marker", "UserDefinedData_t")
    marker.set_data(7)
    zone_a.add_children([coords_a, marker])

    coords_b = Node("Coords", "DataArray_t")
    coords_b.set_data(3)
    zone_b.add_child(coords_b)

    return root

pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target='noder.core.Navigation.child_by_name')
def test_child_by_name():
    # docs:start child_by_name
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_name("Zone_A")
    assert node is not None
    assert node.name() == "Zone_A"
    # docs:end child_by_name


@pytest.mark.doc_example(target='noder.core.Navigation.by_name')
def test_by_name():
    # docs:start by_name
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name("Marker")
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_name


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_name')
def test_all_by_name():
    # docs:start all_by_name
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name("Coords")
    assert len(matches) == 2
    # docs:end all_by_name


@pytest.mark.doc_example(target='noder.core.Navigation.by_name_regex')
def test_by_name_regex():
    # docs:start by_name_regex
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name_regex(r"Zone_\w")
    assert node is not None
    assert node.name().startswith("Zone_")
    # docs:end by_name_regex


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_name_regex')
def test_all_by_name_regex():
    # docs:start all_by_name_regex
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name_regex(r"Zone_\w")
    assert len(matches) == 2
    # docs:end all_by_name_regex


@pytest.mark.doc_example(target='noder.core.Navigation.by_name_glob')
def test_by_name_glob():
    # docs:start by_name_glob
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name_glob("Zone_*")
    assert node is not None
    assert node.name().startswith("Zone_")
    # docs:end by_name_glob


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_name_glob')
def test_all_by_name_glob():
    # docs:start all_by_name_glob
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name_glob("Zone_*")
    assert len(matches) == 2
    # docs:end all_by_name_glob


@pytest.mark.doc_example(target='noder.core.Navigation.child_by_type')
def test_child_by_type():
    # docs:start child_by_type
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_type("Zone_t")
    assert node is not None
    assert node.type() == "Zone_t"
    # docs:end child_by_type


@pytest.mark.doc_example(target='noder.core.Navigation.by_type')
def test_by_type():
    # docs:start by_type
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type("UserDefinedData_t")
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_type


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_type')
def test_all_by_type():
    # docs:start all_by_type
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type("DataArray_t")
    assert len(matches) == 2
    # docs:end all_by_type


@pytest.mark.doc_example(target='noder.core.Navigation.by_type_regex')
def test_by_type_regex():
    # docs:start by_type_regex
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type_regex(r".*Defined.*")
    assert node is not None
    assert node.type() == "UserDefinedData_t"
    # docs:end by_type_regex


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_type_regex')
def test_all_by_type_regex():
    # docs:start all_by_type_regex
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type_regex(r".*Array.*")
    assert len(matches) == 2
    # docs:end all_by_type_regex


@pytest.mark.doc_example(target='noder.core.Navigation.by_type_glob')
def test_by_type_glob():
    # docs:start by_type_glob
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type_glob("*Defined*")
    assert node is not None
    assert node.type() == "UserDefinedData_t"
    # docs:end by_type_glob


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_type_glob')
def test_all_by_type_glob():
    # docs:start all_by_type_glob
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type_glob("*Array*")
    assert len(matches) == 2
    # docs:end all_by_type_glob


@pytest.mark.doc_example(target='noder.core.Navigation.child_by_data')
def test_child_by_data():
    # docs:start child_by_data
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_data("prefix_B")
    assert node is not None
    assert node.name() == "Zone_B"
    # docs:end child_by_data


@pytest.mark.doc_example(target='noder.core.Navigation.by_data')
def test_by_data():
    # docs:start by_data
    tree = _build_navigation_example_tree()
    node = tree.pick().by_data("prefix_A")
    assert node is not None
    assert node.name() == "Zone_A"
    # docs:end by_data


@pytest.mark.doc_example(target='noder.core.Navigation.by_data_glob')
def test_by_data_glob():
    # docs:start by_data_glob
    tree = _build_navigation_example_tree()
    node = tree.pick().by_data_glob("prefix_B")
    assert node is not None
    assert node.name() == "Zone_B"
    # docs:end by_data_glob


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_data')
def test_all_by_data():
    # docs:start all_by_data
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_data(3)
    assert len(matches) == 2
    # docs:end all_by_data


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_data_glob')
def test_all_by_data_glob():
    # docs:start all_by_data_glob
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_data_glob("prefix_*")
    assert len(matches) == 2
    # docs:end all_by_data_glob


@pytest.mark.doc_example(target='noder.core.Navigation.by_and')
def test_by_and():
    # docs:start by_and
    tree = _build_navigation_example_tree()
    node = tree.pick().by_and(name="Marker", type="UserDefinedData_t", data=7)
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_and


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_and')
def test_all_by_and():
    # docs:start all_by_and
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_and(type="DataArray_t", data=3)
    assert len(matches) == 2
    # docs:end all_by_and


@pytest.mark.doc_example(target='noder.core.Navigation.by_and_glob')
def test_by_and_glob():
    # docs:start by_and_glob
    tree = _build_navigation_example_tree()
    node = tree.pick().by_and_glob(name="Zone_*", type="Zone_t", data="prefix_*")
    assert node is not None
    assert node.name() == "Zone_A"
    # docs:end by_and_glob


@pytest.mark.doc_example(target='noder.core.Navigation.all_by_and_glob')
def test_all_by_and_glob():
    # docs:start all_by_and_glob
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_and_glob(name="Zone_*", type="Zone_t", data="prefix_*")
    assert len(matches) == 2
    # docs:end all_by_and_glob


@pytest.mark.doc_example(target='noder.core.Node.get')
@pytest.mark.doc_example(target='noder.core.Node.group')
def test_node_get_group():
    # docs:start node_get_group
    tree = _build_navigation_example_tree()
    node = tree.get(name="Zone_*", type="Zone_t", data="prefix_A")
    matches = tree.group(name="Zone_*", type="Zone_t", data="prefix_*")
    assert node is not None
    assert len(matches) == 2
    # docs:end node_get_group
