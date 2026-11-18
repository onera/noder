"""Executable documentation examples."""

import pytest

import numpy as np

from noder.core import Array, factory

import noder.array.data_types as dtypes

pytestmark = pytest.mark.doc


@pytest.mark.doc_example(target='noder.core.Array.__init__')
def test_array_init():
    # docs:start array_init
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    assert array.size() == 3
    # docs:end array_init


@pytest.mark.doc_example(target='noder.core.Array.__getitem__')
def test_array_getitem():
    # docs:start array_getitem
    from noder.core import Array
    import numpy as np

    array = Array(np.array([10, 20, 30], dtype=np.int32))
    view = array[1]
    assert view.getItemAtIndex(0) == 20
    # docs:end array_getitem


@pytest.mark.doc_example(target='noder.core.Array.__setitem__')
def test_array_setitem():
    # docs:start array_setitem
    from noder.core import Array
    import numpy as np

    array = Array(np.array([10, 20, 30], dtype=np.int32))
    array[1] = 99
    assert array.getItemAtIndex(1) == 99
    # docs:end array_setitem


@pytest.mark.doc_example(target='noder.core.Array.dimensions')
def test_array_dimensions():
    # docs:start array_dimensions
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.dimensions() == 2
    # docs:end array_dimensions


@pytest.mark.doc_example(target='noder.core.Array.size')
def test_array_size():
    # docs:start array_size
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.size() == 4
    # docs:end array_size


@pytest.mark.doc_example(target='noder.core.Array.shape')
def test_array_shape():
    # docs:start array_shape
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert array.shape() == [2, 2]
    # docs:end array_shape


@pytest.mark.doc_example(target='noder.core.Array.strides')
def test_array_strides():
    # docs:start array_strides
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2], [3, 4]], dtype=np.int32))
    assert len(array.strides()) == 2
    # docs:end array_strides


@pytest.mark.doc_example(target='noder.core.Array.getItemAtIndex')
def test_array_get_item_at_index():
    # docs:start array_get_item_at_index
    from noder.core import Array
    import numpy as np

    array = Array(np.array([5, 6, 7], dtype=np.int32))
    assert array.getItemAtIndex(2) == 7
    # docs:end array_get_item_at_index


@pytest.mark.doc_example(target='noder.core.Array.getPyArray')
def test_array_get_py_array():
    # docs:start array_get_py_array
    from noder.core import Array
    import numpy as np

    vector = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    array = Array(vector)
    assert np.array_equal(array.getPyArray(), vector)
    # docs:end array_get_py_array


@pytest.mark.doc_example(target='noder.core.Array.isNone')
def test_array_is_none():
    # docs:start array_is_none
    from noder.core import Array

    array = Array()
    assert array.isNone()
    # docs:end array_is_none


@pytest.mark.doc_example(target='noder.core.Array.isScalar')
def test_array_is_scalar():
    # docs:start array_is_scalar
    from noder.core import Array

    scalar = Array(3.14)
    assert scalar.isScalar()
    # docs:end array_is_scalar


@pytest.mark.doc_example(target='noder.core.Array.isContiguous')
def test_array_is_contiguous():
    # docs:start array_is_contiguous
    from noder.core import factory

    array = factory.zeros_int32([2, 3], "C")
    assert array.isContiguous()
    assert array.isContiguousInStyleC()
    # docs:end array_is_contiguous


@pytest.mark.doc_example(target='noder.core.Array.isContiguousInStyleFortran')
def test_array_is_contiguous_fortran():
    # docs:start array_is_contiguous_fortran
    from noder.core import factory

    array = factory.zeros_int32([2, 3], "F")
    assert array.isContiguous()
    assert array.isContiguousInStyleFortran()
    # docs:end array_is_contiguous_fortran


@pytest.mark.doc_example(target='noder.core.Array.hasString')
def test_array_has_string():
    # docs:start array_has_string
    from noder.core import Array

    array = Array("hello")
    assert array.hasString()
    # docs:end array_has_string


@pytest.mark.doc_example(target='noder.core.Array.extractString')
def test_array_extract_string():
    # docs:start array_extract_string
    from noder.core import Array

    array = Array("hello")
    assert array.extractString() == "hello"
    # docs:end array_extract_string


@pytest.mark.doc_example(target='noder.core.Array.getFlatIndex')
def test_array_get_flat_index():
    # docs:start array_get_flat_index
    from noder.core import Array
    import numpy as np

    array = Array(np.array([[1, 2, 3], [4, 5, 6]], dtype=np.int32))
    assert array.getFlatIndex([1, 2]) == 5
    # docs:end array_get_flat_index


@pytest.mark.doc_example(target='noder.core.Array.getPrintString')
def test_array_get_print_string():
    # docs:start array_get_print_string
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    text = array.getPrintString(40)
    assert isinstance(text, str)
    # docs:end array_get_print_string


@pytest.mark.doc_example(target='noder.core.Array.info')
def test_array_info():
    # docs:start array_info
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    text = array.info()
    assert isinstance(text, str)
    # docs:end array_info


@pytest.mark.doc_example(target='noder.core.Array.print')
def test_array_print():
    # docs:start array_print
    from noder.core import Array
    import numpy as np

    array = Array(np.array([1, 2, 3], dtype=np.int32))
    array.print(40)
    # docs:end array_print
