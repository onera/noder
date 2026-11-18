"""Executable examples for the node factories."""

import pytest

from noder.core import new_node


pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target="noder.core.new_node")
def test_new_node_parent():
    # docs:start new_node_parent
    parent = new_node("parent", "UserDefinedData_t")
    child = new_node("child", "DataArray_t", parent=parent)
    assert child.parent() is parent
    # docs:end new_node_parent
