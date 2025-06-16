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

