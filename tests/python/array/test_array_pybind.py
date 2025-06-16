import pytest
import itertools
import numpy as np
from noder.core import Array


def test_arrayType():
    array = Array()
    assert type(array) == type(Array())


def test_sharingData():
    data = np.array([0,0])
    array1 = Array(data)
    array2 = Array(array1.getPyArray())
    array3 = Array(array2)
    data[0] = 1

    for a in array1, array2, array3:
        assert a.getPyArray()[0] == data[0] == 1

def test_isNone():
    assert (Array()).isNone()
    assert (Array(None)).isNone()
    assert (Array(np.array([]))).isNone()
    assert (Array(np.array([None]))).isNone()


def test_isScalar():
    nullArray = Array()
    assert not nullArray.isScalar()

    scalarArray = Array(np.zeros((1,)))
    assert scalarArray.isScalar()

    vectorArray = Array(np.zeros((2,)))
    assert not vectorArray.isScalar()

    matrixArray = Array(np.zeros((3,3)))
    assert not matrixArray.isScalar()

    stringArray = Array("test string")
    assert not stringArray.isScalar()

def test_extractString():
    array = Array("rho omega")
    assert array.extractString() == "rho omega"

    array = Array("ρω")
    assert array.extractString() == "ρω"

def test_contiguity():
    array = Array(np.zeros((3,)))
    assert array.isContiguousInStyleC()
    assert array.isContiguousInStyleFortran()
    assert array.isContiguous()

    for shape in [(3,3), (3,3,3)]:
        array = Array(np.zeros(shape, order="C"))
        assert array.isContiguousInStyleC()
        assert not array.isContiguousInStyleFortran()
        assert array.isContiguous()
        
        array = Array(np.zeros(shape, order="F"))
        assert not array.isContiguousInStyleC()
        assert array.isContiguousInStyleFortran()
        assert array.isContiguous()

    # TODO include slicing capacity for Array
    matrix = np.zeros((3,3), order="C")
    array = Array(matrix[1:3, 1:3])
    assert not array.isContiguousInStyleC()
    assert not array.isContiguousInStyleFortran()
    assert not array.isContiguous()

@pytest.mark.parametrize("shape", [(3,),(2,2),(3,3,3),(2,3,4),(3,2,4,3)])
@pytest.mark.parametrize("order", ["C", "F"])
def test_getFlatIndex(shape, order):
    for indices in itertools.product(*[range(s) for s in shape]):
        array = Array(np.zeros(shape,order=order))
        assert np.ravel_multi_index(indices,shape, order=order) == array.getFlatIndex(indices)



if __name__ == "__main__":
    test_sharingData()