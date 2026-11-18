"""Executable documentation examples."""

from noder.core import Node

def get_all_paths_list(node, pathlist):
    pathlist += [node.path()]
    for child in node.children():
        get_all_paths_list(child, pathlist)

def get_all_paths(node):
    paths = []
    get_all_paths_list(node, paths)
    return paths

import pytest

pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target='noder.core.Node.__add__')
def test_operator_plus():
    # docs:start node_group_operator_plus
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    c = Node("c")
    _group = b + c
    a / _group
    assert [child.name() for child in a.children()] == ["b", "c"]
    # docs:end node_group_operator_plus


@pytest.mark.doc_example(target='noder.core.Node.__truediv__')
def test_operator_div_chain():
    # docs:start node_group_operator_div_chain
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    c = Node("c")
    end = a / b / c
    assert end is c
    assert c.path() == "a/b/c"
    # docs:end node_group_operator_div_chain


@pytest.mark.doc_example(target='noder.core.Node.__truediv__')
def test_operator_div_group():
    # docs:start node_group_operator_div_group
    from noder.core import Node

    root = Node("root")
    branch = Node("branch")
    leaf_a = Node("leaf_a")
    leaf_b = Node("leaf_b")
    root / branch / (leaf_a + leaf_b)
    assert [child.name() for child in branch.children()] == ["leaf_a", "leaf_b"]
    # docs:end node_group_operator_div_group


@pytest.mark.doc_example(target='noder.core._NodeGroup.nodes')
def test_node_group_nodes():
    # docs:start node_group_nodes
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    group = a + b
    assert [node.name() for node in group.nodes()] == ["a", "b"]
    # docs:end node_group_nodes
