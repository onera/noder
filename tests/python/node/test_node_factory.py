import numpy as np
from noder.core import new_node

def test_new_node_no_args():
    node = new_node()

def test_new_node_only_name():
    node = new_node("TheName")
    assert node.name() == "TheName"

def test_new_node_name_and_type():
    node = new_node("TheName", "TheType")
    assert node.name() == "TheName"
    assert node.type() == "TheType"

def test_new_node_data_int():
    node = new_node("TheName", "TheType",1)

    data = node.data().getPyArray()
    assert data == 1

def test_new_node_data_str():
    node = new_node("TheName", "TheType", "MyData")

    data = node.data().extractString()
    assert data == "MyData"

def test_new_node_data_numpy_direct():
    node = new_node("TheName", "TheType", np.linspace(0,1,11))

    data = node.data().getPyArray()
    assert len(data) == 11

def test_new_node_data_numpy_shared_check():
    vector = np.linspace(0,1,11)
    node = new_node("TheName", "TheType", vector)

    data = node.data().getPyArray()
    assert data is vector

def test_new_node_parent():
    a = new_node("a")
    b = new_node("b", parent=a)
    b_picked = a.pick().child_by_name("b")

    assert b is b_picked



if __name__ == '__main__':
    test_new_node_data_int()