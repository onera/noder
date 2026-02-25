import pytest
import noder.tests.array as test_in_cpp
import noder.array.data_types as dtypes

def test_constructorEmpty(): return test_in_cpp.constructorEmpty()
def test_constructorPyArray(): return test_in_cpp.constructorPyArray()
def test_constructorString(): return test_in_cpp.constructorString()
def test_constructorAnotherArray(): return test_in_cpp.constructorAnotherArray()

def test_getArrayProperties(): return test_in_cpp.getArrayProperties()
def test_getFlatIndex(): return test_in_cpp.getFlatIndex()
def test_getPyArray(): return test_in_cpp.getPyArray()
def test_extractString(): return test_in_cpp.extractString()
def test_getPrintStringAndInfo(): return test_in_cpp.getPrintStringAndInfo()

def test_sharingData(): return test_in_cpp.sharingData()

def test_arrayWithStringHasStringTrue(): return test_in_cpp.arrayWithStringHasStringTrue()
def test_arrayWithUnicodeStringHasStringTrue(): return test_in_cpp.arrayWithUnicodeStringHasStringTrue()
def test_arrayWithNumbersHasStringFalse(): return test_in_cpp.arrayWithNumbersHasStringFalse()


def test_isNone(): return test_in_cpp.isNone()
# def test_isNoneWithNoneObj(): return test_in_cpp.isNoneWithNoneObj()
def test_arrayWithNumbersIsNotNone(): return test_in_cpp.arrayWithNumbersIsNotNone()
def test_arrayWithStringIsNotNone(): return test_in_cpp.arrayWithStringIsNotNone()
def test_arrayWithNumberOfSizeZeroIsNone(): return test_in_cpp.arrayWithNumberOfSizeZeroIsNone()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_isScalar(dtype):
    return getattr(test_in_cpp,f"isScalar_{dtype}")()



@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_contiguity(dtype):
    return getattr(test_in_cpp,f"contiguity_{dtype}")()

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_hasDataOfType(dtype):
    return getattr(test_in_cpp,f"hasDataOfType_{dtype}")()

def test_doNotHaveDataOfType(): test_in_cpp.doNotHaveDataOfType()


if __name__ == "__main__":
    test_constructorEmpty()
