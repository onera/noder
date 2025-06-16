import pytest
import noder.tests.array as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_setArrayToScalar(dtype):
    return getattr(test_in_cpp,f"setArrayToScalar_{dtype}")()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_setFromArrayConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"setFromArrayConsideringAllTypes_{dtype}")()

def test_setFromArrayToRange(): return test_in_cpp.setFromArrayToRange()


def test_catchErrorWhenAssigningWrongScalarType():
    return test_in_cpp.catchErrorWhenAssigningWrongScalarType()


def test_catchErrorWhenAssigningScalarToStringArray():
    return test_in_cpp.catchErrorWhenAssigningScalarToStringArray()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_addScalarConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"addScalarConsideringAllTypes_{dtype}")()


def test_addScalarToRange(): return test_in_cpp.addScalarToRange()

def test_substractScalarToRange(): return test_in_cpp.substractScalarToRange()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_multiplyScalarConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"multiplyScalarConsideringAllTypes_{dtype}")()


def test_multiplyScalarToRange(): return test_in_cpp.multiplyScalarToRange()
def test_divideScalarToRangeOfIntegers(): return test_in_cpp.divideScalarToRangeOfIntegers()
def test_divideScalarToRangeOfFloats(): return test_in_cpp.divideScalarToRangeOfFloats()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_addFromArrayConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"addFromArrayConsideringAllTypes_{dtype}")()


def test_addFromArrayToRange(): return test_in_cpp.addFromArrayToRange()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_substractFromArrayConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"substractFromArrayConsideringAllTypes_{dtype}")()

def test_substractFromArrayToRange(): return test_in_cpp.substractFromArrayToRange()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_multiplyFromArrayConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"multiplyFromArrayConsideringAllTypes_{dtype}")()

def test_multiplyFromArrayToRange(): return test_in_cpp.multiplyFromArrayToRange()



@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_divideFromArrayConsideringAllTypes(dtype):
    return getattr(test_in_cpp,f"divideFromArrayConsideringAllTypes_{dtype}")()

def test_divideFromArrayToRange(): return test_in_cpp.divideFromArrayToRange()
