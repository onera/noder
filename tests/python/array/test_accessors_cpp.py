import pytest
import noder.tests.array as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_scalarSlicingProducesScalar(dtype):
    return getattr(test_in_cpp,f"scalarSlicingProducesScalar_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_scalarSlicingDoesNotMakeCopy(dtype):
    return getattr(test_in_cpp,f"scalarSlicingDoesNotMakeCopy_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_getItemAtIndex(dtype):
    return getattr(test_in_cpp,f"getItemAtIndex_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_getPointerOfDataSafely(dtype):
    return getattr(test_in_cpp,f"getPointerOfDataSafely_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_getPointerOfModifiableDataFast(dtype):
    return getattr(test_in_cpp,f"getPointerOfModifiableDataFast_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.scalar_types)
def test_getPointerOfReadOnlyDataFast(dtype):
    return getattr(test_in_cpp,f"getPointerOfReadOnlyDataFast_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_getAccessorOfReadOnlyData(dtype):
    return getattr(test_in_cpp,f"getAccessorOfReadOnlyData_{dtype}")()


@pytest.mark.parametrize("dtype", dtypes.floating_and_integral_types)
def test_getAccessorOfModifiableData(dtype):
    return getattr(test_in_cpp,f"getAccessorOfModifiableData_{dtype}")()


def test_getFlatIndexOfArrayInStyleC(): return test_in_cpp.getFlatIndexOfArrayInStyleC()


def test_getFlatIndexOfArrayInStyleFortran(): return test_in_cpp.getFlatIndexOfArrayInStyleFortran()


def test_extractStringAscii(): return test_in_cpp.extractStringAscii()


def test_extractStringUnicode(): return test_in_cpp.extractStringUnicode()