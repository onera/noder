import pytest
import numpy as np
from noder.core import Array, factory
import noder.array.data_types as dtypes

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_scalar_slicing_produces_scalar(dtype):
    matrix_builder = getattr(factory,f"zeros_{dtype}")
    array = matrix_builder([2],'C')
    item = array[0]
    assert item.isScalar()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_scalar_slicing_does_not_make_copy(dtype):
    matrix_builder = getattr(factory,f"zeros_{dtype}")
    array = matrix_builder([2],'C')
    
    newValue = 1
    item_no_copy = array[0:1] # BEWARE does not make copy (same behavior as numpy)
    item_copy = array[0] # BEWARE makes copy (same behavior as numpy)
    array[0] = newValue

    assert isinstance(item_copy, Array)
    assert isinstance(item_no_copy, Array)

    assert item_no_copy.getPyArray() == newValue
    assert item_copy.getPyArray() == 0


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_get_item_at_index(dtype):
    array = Array(np.array([0,1,0], dtype=dtypes.to_numpy_dtype[dtype]))

    first_item = array.getItemAtIndex(0)

    assert first_item == 0
    
    array[0] = 1
    assert array[0].getPyArray() != first_item # BEWARE as opposed to C++, in Python a copy is made

def test_get_flat_index_order_c():
    array = factory.empty_int8((2,3,4),'C')
    flat_index_23 = array.getFlatIndex((1,2,3))
    assert flat_index_23 == 23

    flat_index_6 = array.getFlatIndex((0,1,2))
    assert flat_index_6 == 6

def test_get_flat_index_order_f():
    array = factory.empty_int8((2,3,4),'F')
    flat_index_23 = array.getFlatIndex((1,2,3))
    assert flat_index_23 == 23

    flat_index_14 = array.getFlatIndex((0,1,2))
    assert flat_index_14 == 14

def test_extract_string():
    assert Array(np.array("test")).extractString() == "test"
    assert Array(np.array(b"test")).extractString() == "test"
    assert Array(np.array(["test string"])).extractString() == "test string"
    assert Array(np.array(["ρω"])).extractString() == "ρω"
    assert Array(np.array(["💩"])).extractString() == "💩"
    assert Array(np.array([0,1,2])).extractString() == ""
    assert Array(np.array([])).extractString() == ""


if __name__ == '__main__':
    test_get_item_at_index("float")