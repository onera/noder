from noder.core import Array, Node


def test_data_is_scalar():
    node = Node("value")
    node.set_data(12)
    assert node.data().isScalar()


def test_data_extract_string():
    payload = Array("abc")
    node = Node("node")
    node.set_data(payload)
    assert node.data().extractString() == "abc"
