import pytest
import numpy as np
from noder.core import Array, factory
import noder.array.data_types as dtypes

def assert_contiguity_coherent(array, order):
    npy_array = array.getPyArray()
    if len(npy_array.shape)== 1:
        assert npy_array.flags['C_CONTIGUOUS']
        assert npy_array.flags['F_CONTIGUOUS']

    elif order == 'C':
        assert npy_array.flags['C_CONTIGUOUS']
        assert not npy_array.flags['F_CONTIGUOUS']
    
    elif order == 'F':
        assert not npy_array.flags['C_CONTIGUOUS']
        assert npy_array.flags['F_CONTIGUOUS']
    
    else:
        raise ValueError('contiguity incoherency')

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
@pytest.mark.parametrize("order", ['C','F'])
@pytest.mark.parametrize("shape",[[5],[3,2],[4,3,2]])
def test_zeros(dtype,order,shape):
    
    # matrix_builder = getattr(factory,f"zeros_{dtype}")
    # array = matrix_builder(shape,order)
    array = factory.zeros(shape,order,dtype=dtype)

    expected = np.zeros(shape,order=order)

    assert_contiguity_coherent(array,order)
    assert np.allclose(array.getPyArray(), expected)
    assert array.size() == np.prod(shape)

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
@pytest.mark.parametrize("order", ['C','F'])
@pytest.mark.parametrize("shape",[[5],[3,2],[4,3,2]])
def test_ones(dtype,order,shape):
    matrix_builder = getattr(factory,f"ones_{dtype}")
    
    array = matrix_builder(shape,order)

    expected = np.ones(shape,order=order)

    assert_contiguity_coherent(array,order)
    assert np.allclose(array.getPyArray(), expected)
    assert array.size() == np.prod(shape)

@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
@pytest.mark.parametrize("order", ['C','F'])
@pytest.mark.parametrize("shape",[[5],[3,2],[4,3,2]])
def test_full(dtype,order,shape):
    matrix_builder = getattr(factory,f"full_{dtype}")
    
    value = 2
    array = matrix_builder(shape, value, order)

    expected = np.full(shape, value,order=order)

    assert_contiguity_coherent(array,order)
    assert np.allclose(array.getPyArray(), expected)
    assert array.size() == np.prod(shape)

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
@pytest.mark.parametrize("order", ['C','F'])
@pytest.mark.parametrize("shape",[[5],[3,2],[4,3,2]])
def test_empty(dtype,order,shape):
    matrix_builder = getattr(factory,f"empty_{dtype}")
    
    array = matrix_builder(shape,order)

    assert_contiguity_coherent(array,order)
    assert array.size() == np.prod(shape)

if __name__ == '__main__':
    test_zeros("float")