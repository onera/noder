from noder.core import Array, Node


def test_data_has_string_example():
    # docs:start data_has_string_example
    from noder.core import Node

    node = Node("message")
    node.set_data("hello")
    assert node.data().hasString()
    # docs:end data_has_string_example


def test_data_is_none_example():
    # docs:start data_is_none_example
    from noder.core import Array

    data = Array()
    assert data.isNone()
    # docs:end data_is_none_example


def test_data_is_scalar_example():
    # docs:start data_is_scalar_example
    from noder.core import Node

    node = Node("value")
    node.set_data(12)
    assert node.data().isScalar()
    # docs:end data_is_scalar_example


def test_data_extract_string_example():
    # docs:start data_extract_string_example
    from noder.core import Node

    node = Node("message")
    node.set_data("hello")
    assert node.data().extractString() == "hello"
    # docs:end data_extract_string_example


def test_data_is_scalar():
    node = Node("value")
    node.set_data(12)
    assert node.data().isScalar()


def test_data_extract_string():
    payload = Array("abc")
    node = Node("node")
    node.set_data(payload)
    assert node.data().extractString() == "abc"
