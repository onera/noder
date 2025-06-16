import pytest
import numpy as np
from noder.core import Array, factory
import noder.array.data_types as dtypes

def test_arrayFromString():
    test_string = "test string"
    result = factory.arrayFromString(test_string)

    assert result.hasString()
    assert result.size() == 1
    assert result.extractString() == test_string

def test_arrayFromUnicodeString():

    test_string = "ρω"
    result = factory.arrayFromString(test_string)

    assert result.hasString()
    assert result.size() == 1
    assert result.extractString() == test_string
