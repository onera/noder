import pytest
from noder.tests.array import factory as test_in_cpp
import noder.array.data_types as dtypes

def test_arrayFromString(): return test_in_cpp.arrayFromString()

def test_arrayFromUnicodeString(): return test_in_cpp.arrayFromUnicodeString()