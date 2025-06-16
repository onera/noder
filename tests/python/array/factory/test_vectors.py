import pytest
import numpy as np
from noder.core import Array, factory
import noder.array.data_types as dtypes


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_uniformFromStep_positive_step(dtype):
    uniform_builder = getattr(factory,f"uniformFromStep_{dtype}")
    result = uniform_builder(0,10,2)
    
    expected = np.array([0,2,4,6,8],dtype=dtypes.to_numpy_dtype[dtype])

    assert result.size() == expected.size
    assert np.allclose(result.getPyArray(),expected)


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_uniformFromStep_negative_step(dtype):
    uniform_builder = getattr(factory,f"uniformFromStep_{dtype}")
    result = uniform_builder(10,0,-2)
    
    expected = np.array([10,8,6,4,2],dtype=dtypes.to_numpy_dtype[dtype])

    assert result.size() == expected.size
    assert np.allclose(result.getPyArray(),expected)


def test_uniformFromStep_zero_step():

    try:
        result = factory.uniformFromStep_int32(0,10,0)
    except ValueError as e:
        assert str(e) == "step must not be zero"


def test_uniformFromStep_incoherent_step():

    try:
        result = factory.uniformFromStep_int32(10,0,1)
    except ValueError as e:
        assert str(e) == "incoherent set of start, stop, step"


@pytest.mark.parametrize("dtype", dtypes.floating_types)
def test_uniformFromCount_true(dtype):
    count = 5
    uniform_builder = getattr(factory,f"uniformFromCount_{dtype}")
    result = uniform_builder(0, 1, count, True)

    expected = np.array([0.0, 0.25, 0.50, 0.75, 1.0],dtype=dtypes.to_numpy_dtype[dtype])

    assert np.allclose(result.getPyArray(), expected)
    

@pytest.mark.parametrize("dtype", dtypes.floating_types)
def test_uniformFromCount_false(dtype):
    count = 5
    uniform_builder = getattr(factory,f"uniformFromCount_{dtype}")
    result = uniform_builder(0, 1, count, False)

    expected = np.array([0, 0.2, 0.4, 0.6, 0.8],dtype=dtypes.to_numpy_dtype[dtype])

    assert np.allclose(result.getPyArray(), expected)

@pytest.mark.parametrize("dtype", dtypes.floating_types)
def test_uniformFromCount_zero(dtype):
    uniform_builder = getattr(factory,f"uniformFromCount_{dtype}")

    try:
        result = uniform_builder(0,1,0,True)
    except ValueError as e:
        assert str(e) == "num must be at least 2"


@pytest.mark.parametrize("dtype", dtypes.floating_types)
def test_uniformFromCount_one(dtype):
    uniform_builder = getattr(factory,f"uniformFromCount_{dtype}")

    try:
        result = uniform_builder(0,1,1,True)
    except ValueError as e:
        assert str(e) == "num must be at least 2"


@pytest.mark.parametrize("dtype", dtypes.floating_types)
def test_uniformFromCount_floating_point_values(dtype):
    uniform_builder = getattr(factory,f"uniformFromCount_{dtype}")
    
    count = 5
    result = uniform_builder(0,2,count,True)

    expected = np.array([0, 0.5, 1.0, 1.5, 2.0],dtype=dtypes.to_numpy_dtype[dtype])
    assert np.allclose(result.getPyArray(), expected)
