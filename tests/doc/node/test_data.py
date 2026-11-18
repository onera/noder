"""Executable documentation examples."""

from noder.core import Array, Node

import pytest

pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target='noder.core.Data.hasString')
def test_data_has_string():
    # docs:start data_has_string
    from noder.core import Node

    node = Node("message")
    node.set_data("hello")
    assert node.data().hasString()
    # docs:end data_has_string


@pytest.mark.doc_example(target='noder.core.Data.isNone')
def test_data_is_none():
    # docs:start data_is_none
    from noder.core import Array

    data = Array()
    assert data.isNone()
    # docs:end data_is_none


@pytest.mark.doc_example(target='noder.core.Data.isScalar')
def test_data_is_scalar():
    # docs:start data_is_scalar
    from noder.core import Node

    node = Node("value")
    node.set_data(12)
    assert node.data().isScalar()
    # docs:end data_is_scalar


@pytest.mark.doc_example(target='noder.core.Data.extractString')
def test_data_extract_string():
    # docs:start data_extract_string
    from noder.core import Node

    node = Node("message")
    node.set_data("hello")
    assert node.data().extractString() == "hello"
    # docs:end data_extract_string
