import pytest
from noder.tests.array import factory as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("typestr", dtypes.scalar_types)
@pytest.mark.parametrize("mode", ['positive_step','negative_step'])
def test_uniformFromCount(mode, typestr):
    
    if typestr == "bool": return
    test_function = getattr(test_in_cpp,f"uniformFromCount_{mode}_{typestr}")

    try:
        test_function()
    except ValueError as e:
        if typestr.startswith("u"):
            if "incoherent set of start, stop, step" in str(e):
                return
        raise ValueError("unexpected ValueError, see traceback") from e

def test_uniformFromCount_zero_step(): return test_in_cpp.uniformFromCount_zero_step()

def test_uniformFromCount_incoherent_step(): return test_in_cpp.uniformFromCount_incoherent_step()


@pytest.mark.parametrize("typestr", dtypes.floating_types)
def test_uniformFromCount_endpoint_true(typestr):
    test_function = getattr(test_in_cpp,f"uniformFromCount_endpoint_true_{typestr}")
    test_function()

@pytest.mark.parametrize("typestr", dtypes.floating_types)
def test_uniformFromCount_endpoint_false(typestr):
    test_function = getattr(test_in_cpp,f"uniformFromCount_endpoint_false_{typestr}")
    test_function()

@pytest.mark.parametrize("typestr", dtypes.floating_types)
def test_uniformFromCount_num_zero(typestr):
    test_function = getattr(test_in_cpp,f"uniformFromCount_num_zero_{typestr}")
    test_function()

@pytest.mark.parametrize("typestr", dtypes.floating_types)
def test_uniformFromCount_num_one(typestr):
    test_function = getattr(test_in_cpp,f"uniformFromCount_num_one_{typestr}")
    test_function()

@pytest.mark.parametrize("typestr", dtypes.floating_types)
def test_uniformFromCount_floating_point_values(typestr):
    test_function = getattr(test_in_cpp,f"uniformFromCount_floating_point_values_{typestr}")
    test_function()
