import pytest
import noder.tests.array as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_twoIdenticalArraysAreEqual(dtype):
    return getattr(test_in_cpp,f"twoIdenticalArraysAreEqual_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual(dtype):
    return getattr(test_in_cpp,f"twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_twoArraysWithDifferentItemsAreNotEqual(dtype):
    return getattr(test_in_cpp,f"twoArraysWithDifferentItemsAreNotEqual_{dtype}")()

def test_twoIdenticalArraysButWithDifferentDataTypesAreEqual():
    return test_in_cpp.twoIdenticalArraysButWithDifferentDataTypesAreEqual()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_twoArraysWithDifferentSizesAreNotEqual(dtype):
    return getattr(test_in_cpp,f"twoArraysWithDifferentSizesAreNotEqual_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_twoArraysWithDifferentShapesButSameSizeAreEqual(dtype):
    return getattr(test_in_cpp,f"twoArraysWithDifferentShapesButSameSizeAreEqual_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_arrayOfZerosIsEqualToScalarZero(dtype):
    return getattr(test_in_cpp,f"arrayOfZerosIsEqualToScalarZero_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_arrayOfZerosIsDifferentToScalarOne(dtype):
    return getattr(test_in_cpp,f"arrayOfZerosIsDifferentToScalarOne_{dtype}")()


def test_rangeIsNeverEqualToScalar():
    return test_in_cpp.rangeIsNeverEqualToScalar()


def test_twoIdenticalArraysContainingStringsAreEqual():
    return test_in_cpp.twoIdenticalArraysContainingStringsAreEqual()

def test_twoIdenticalArraysContainingUnicodeStringsAreEqual():
    return test_in_cpp.twoIdenticalArraysContainingUnicodeStringsAreEqual()

def test_arrayEqualToString():
    return test_in_cpp.arrayEqualToString()

def test_arrayEqualToUnicodeString():
    return test_in_cpp.arrayEqualToUnicodeString()

def test_arrayDifferentToString():
    return test_in_cpp.arrayDifferentToString()

def test_numericalArrayDifferentToString():
    return test_in_cpp.numericalArrayDifferentToString()





