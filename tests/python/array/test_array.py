import pytest
import numpy as np
from noder.core import Array, factory
import noder.array.data_types as dtypes


def test_constructorEmpty():
    array = Array()

def test_constructorPyArray():
    pyarray = np.array([1,2,3],dtype=int)
    array = Array(pyarray)

def test_constructorPyNone():
    array = Array(None)

def test_constructorString():
    array = Array("test string")

def test_constructorAnotherArray():
    other = Array("test string")
    array = Array(other)

def test_getArrayProperties():
    array = factory.zeros_int8([3,3], 'C')
    
    assert array.dimensions() == 2
    assert array.size() == 9
    assert array.shape()[0] == 3
    assert array.shape()[1] == 3
    assert len(array.strides()) == 2

def test_sharingData():
    pyarray = np.array([0,0], dtype=np.int32)
    array0 = Array(pyarray)
    array1 = array0
    array2 = Array(array1)

    pyarray[0] = 1
    for array in (array0, array1, array2):
        assert array.getItemAtIndex(0) == 1

    secondItem = array1.getItemAtIndex(1) # note this makes copy in Python
    pyarray[1] = 2

    assert secondItem != pyarray[1]

def test_arrayWithStringHasStringTrue():
    array = Array("test string")
    assert array.hasString()

def test_arrayWithUnicodeStringHasStringTrue():
    array = Array("Λουίς")
    assert array.hasString()

def test_arrayEmptyHasStringFalse():
    array = Array()
    assert not array.hasString()

def test_arrayWithNumbersHasStringFalse():
    array = factory.uniformFromStep_int8(0.0,5.0,1.0)
    assert not array.hasString()

def test_isNone():
    array = Array()
    assert array.isNone()

def test_arrayWithNumbersIsNotNone():
    array = factory.zeros_int8((2,),'C')
    assert not array.isNone()

def test_arrayWithStringIsNotNone():
    array = Array("test string")
    assert not array.isNone()

def test_arrayWithNumberOfSizeZeroIsNone():
    array = factory.zeros_int8((0,),'C')
    assert array.isNone()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_isScalar(dtype):

    nullArray = Array()
    assert not nullArray.isScalar()

    zeros_builder = getattr(factory,f"zeros_{dtype}")
    zeroSizeArray = zeros_builder([0],'C')
    assert not zeroSizeArray.isScalar()

    scalarArray = zeros_builder([1],'C')
    assert scalarArray.isScalar()

    directBooleanArray = Array(True)
    assert directBooleanArray.isScalar()

    directIntegerArray = Array(12)
    assert directIntegerArray.isScalar()

    directFloatArray = Array(3.14159)
    assert directFloatArray.isScalar()

    vectorArray = zeros_builder([2],'C')
    assert not vectorArray.isScalar()

    matrixArray = zeros_builder([3,3],'C')
    assert not matrixArray.isScalar()

def test_isScalar_string():
    stringArray = Array("test string")
    assert not stringArray.isScalar()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
@pytest.mark.parametrize("order", ['C','F'])
@pytest.mark.parametrize("dims", range(1,4))
def test_contiguity(dtype, order, dims):
    zeros_builder = getattr(factory,f"zeros_{dtype}")
    array = zeros_builder([3]*dims,order)
    
    if dims < 2:
        assert array.isContiguousInStyleC()
        assert array.isContiguousInStyleFortran()
        assert array.isContiguous()

    elif order=='C':
        assert array.isContiguousInStyleC()
        assert not array.isContiguousInStyleFortran()
        assert array.isContiguous()

    else:
        assert not array.isContiguousInStyleC()
        assert array.isContiguousInStyleFortran()
        assert array.isContiguous()

def test_nonContiguous():
    array = factory.zeros_int8((3,3), 'C')
    non_contiguous_pyarray = array.getPyArray()[1:3,1:3]
    subarray = Array(non_contiguous_pyarray)
    assert not subarray.isContiguous()


def test_array_init_example():
    # docs:start array_init_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    assert array.size() == 3
    # docs:end array_init_example


def test_array_getitem_example():
    # docs:start array_getitem_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([10, 20, 30], dtype=np.int32))
    view = array[1]
    assert view.getItemAtIndex(0) == 20
    # docs:end array_getitem_example


def test_array_setitem_example():
    # docs:start array_setitem_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([10, 20, 30], dtype=np.int32))
    array[1] = 99
    assert array.getItemAtIndex(1) == 99
    # docs:end array_setitem_example


def test_array_dimensions_example():
    # docs:start array_dimensions_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.dimensions() == 2
    # docs:end array_dimensions_example


def test_array_size_example():
    # docs:start array_size_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.size() == 4
    # docs:end array_size_example


def test_array_shape_example():
    # docs:start array_shape_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.shape() == [2, 2]
    # docs:end array_shape_example


def test_array_strides_example():
    # docs:start array_strides_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert len(array.strides()) == 2
    # docs:end array_strides_example


def test_array_get_item_at_index_example():
    # docs:start array_get_item_at_index_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([5, 6, 7], dtype=np.int32))
    assert array.getItemAtIndex(2) == 7
    # docs:end array_get_item_at_index_example


def test_array_get_py_array_example():
    # docs:start array_get_py_array_example
    from noder.core import Array
    import numpy as np

    vector = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    array = Array(vector)
    assert np.array_equal(array.getPyArray(), vector)
    # docs:end array_get_py_array_example


def test_array_is_none_example():
    # docs:start array_is_none_example
    from noder.core import Array

    array = Array()
    assert array.isNone()
    # docs:end array_is_none_example


def test_array_is_scalar_example():
    # docs:start array_is_scalar_example
    from noder.core import Array

    scalar = Array(3.14)
    assert scalar.isScalar()
    # docs:end array_is_scalar_example


def test_array_is_contiguous_example():
    # docs:start array_is_contiguous_example
    from noder.core import factory

    array = factory.zeros_int32([2, 3], "C")
    assert array.isContiguous()
    assert array.isContiguousInStyleC()
    # docs:end array_is_contiguous_example


def test_array_is_contiguous_fortran_example():
    # docs:start array_is_contiguous_fortran_example
    from noder.core import factory

    array = factory.zeros_int32([2, 3], "F")
    assert array.isContiguous()
    assert array.isContiguousInStyleFortran()
    # docs:end array_is_contiguous_fortran_example


def test_array_has_string_example():
    # docs:start array_has_string_example
    from noder.core import Array

    array = Array("hello")
    assert array.hasString()
    # docs:end array_has_string_example


def test_array_extract_string_example():
    # docs:start array_extract_string_example
    from noder.core import Array

    array = Array("hello")
    assert array.extractString() == "hello"
    # docs:end array_extract_string_example


def test_array_get_flat_index_example():
    # docs:start array_get_flat_index_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2, 3], [4, 5, 6]], dtype=np.int32))
    assert array.getFlatIndex([1, 2]) == 5
    # docs:end array_get_flat_index_example


def test_array_get_print_string_example():
    # docs:start array_get_print_string_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    text = array.getPrintString(40)
    assert isinstance(text, str)
    # docs:end array_get_print_string_example


def test_array_info_example():
    # docs:start array_info_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    text = array.info()
    assert isinstance(text, str)
    # docs:end array_info_example


def test_array_print_example():
    # docs:start array_print_example
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    array.print(40)
    # docs:end array_print_example

