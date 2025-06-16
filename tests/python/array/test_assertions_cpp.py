import pytest
import noder.tests.array as test_in_cpp
import noder.array.data_types as dtypes

def test_assertSameSizeAsVector(): return test_in_cpp.assertSameSizeAsVector()
def test_assertSameSizeAsArray(): return test_in_cpp.assertSameSizeAsArray()
def test_assertSameSizeAsPyArray(): return test_in_cpp.assertSameSizeAsPyArray()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_mustHaveDataOfTypeAndDimensions(dtype):
    return getattr(test_in_cpp,f"mustHaveDataOfTypeAndDimensions_{dtype}")()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_mustHaveDataOfType(dtype):
    return getattr(test_in_cpp,f"mustHaveDataOfType_{dtype}")()

def test_mustHaveDataOfTypeCatchExpectedError():
    return test_in_cpp.mustHaveDataOfTypeCatchExpectedError()

def test_mustHaveDataOfDimensions():
    return test_in_cpp.mustHaveDataOfDimensions()

def test_mustHaveDataOfDimensionsCatchExpectedError():
    return test_in_cpp.mustHaveDataOfDimensionsCatchExpectedError()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_mustHaveValidDataTypeForSettingScalar(dtype):
    return getattr(test_in_cpp,f"mustHaveValidDataTypeForSettingScalar_{dtype}")()

def test_mustHaveValidDataTypeForSettingScalarCatchExpectedError():
    return test_in_cpp.mustHaveValidDataTypeForSettingScalarCatchExpectedError()
