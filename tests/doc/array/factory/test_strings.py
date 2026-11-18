"""Executable examples for string array factories."""

import pytest

from noder.core import factory


pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target="noder.core.factory.arrayFromString")
def test_array_from_string():
    # docs:start array_from_string
    text = "blade section"
    payload = factory.arrayFromString(text)
    assert payload.hasString()
    assert payload.extractString() == text
    # docs:end array_from_string


@pytest.mark.doc_example(target="noder.core.factory.arrayFromUnicodeString")
def test_array_from_unicode_string():
    # docs:start array_from_unicode_string
    text = "ρω"
    payload = factory.arrayFromUnicodeString(text)
    assert payload.hasString()
    assert payload.extractString() == text
    # docs:end array_from_unicode_string
