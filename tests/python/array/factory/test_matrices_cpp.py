import pytest
from noder.tests.array import factory as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("typestr", dtypes.floating_and_integral_types)
@pytest.mark.parametrize("order", ["c","f"])
@pytest.mark.parametrize("function_name", ["empty", "full", "zeros", "ones"])
def test_matrices(function_name, order, typestr):
    test_function = getattr(test_in_cpp,f"{function_name}_{order}_order_{typestr}")
    test_function()

@pytest.mark.parametrize("typestr", dtypes.floating_and_integral_types)
@pytest.mark.parametrize("order", ["c","f"])
def test_empty(typestr, order):
    test_function = getattr(test_in_cpp,f"empty_{order}_order_{typestr}")
    test_function()

