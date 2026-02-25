import pytest
import noder.array.data_types as dtypes


def test_raise_error_if_invalid_dtype_accepts_known_type():
    dtypes.raise_error_if_invalid_dtype("int32")


def test_raise_error_if_invalid_dtype_rejects_unknown_type():
    with pytest.raises(ValueError):
        dtypes.raise_error_if_invalid_dtype("not_a_dtype")


def test_raise_error_if_invalid_order_accepts_known_order():
    dtypes.raise_error_if_invalid_order("C")
    dtypes.raise_error_if_invalid_order("F")


def test_raise_error_if_invalid_order_rejects_unknown_order():
    with pytest.raises(ValueError):
        dtypes.raise_error_if_invalid_order("X")
