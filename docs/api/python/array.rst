Array (Python)
==============

.. currentmodule:: noder.core

Python ``Array`` wraps NumPy arrays with the noder ``Data`` interface.

C++ counterpart: :doc:`../cpp/array`

Class
-----

.. autoclass:: Array
   :member-order: bysource

Methods
---------------

``__init__``
~~~~~~~~~~~~

.. automethod:: Array.__init__

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_init_example
   :end-before: # docs:end array_init_example
   :dedent: 4

``__getitem__``
~~~~~~~~~~~~~~~

.. automethod:: Array.__getitem__

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_getitem_example
   :end-before: # docs:end array_getitem_example
   :dedent: 4

``__setitem__``
~~~~~~~~~~~~~~~

.. automethod:: Array.__setitem__

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_setitem_example
   :end-before: # docs:end array_setitem_example
   :dedent: 4

``dimensions``
~~~~~~~~~~~~~~

.. automethod:: Array.dimensions

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_dimensions_example
   :end-before: # docs:end array_dimensions_example
   :dedent: 4

``size``
~~~~~~~~

.. automethod:: Array.size

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_size_example
   :end-before: # docs:end array_size_example
   :dedent: 4

``shape``
~~~~~~~~~

.. automethod:: Array.shape

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_shape_example
   :end-before: # docs:end array_shape_example
   :dedent: 4

``strides``
~~~~~~~~~~~

.. automethod:: Array.strides

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_strides_example
   :end-before: # docs:end array_strides_example
   :dedent: 4

``getItemAtIndex``
~~~~~~~~~~~~~~~~~~

.. automethod:: Array.getItemAtIndex

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_get_item_at_index_example
   :end-before: # docs:end array_get_item_at_index_example
   :dedent: 4

``getPyArray``
~~~~~~~~~~~~~~

.. automethod:: Array.getPyArray

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_get_py_array_example
   :end-before: # docs:end array_get_py_array_example
   :dedent: 4

``isNone``
~~~~~~~~~~

.. automethod:: Array.isNone

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_is_none_example
   :end-before: # docs:end array_is_none_example
   :dedent: 4

``isScalar``
~~~~~~~~~~~~

.. automethod:: Array.isScalar

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_is_scalar_example
   :end-before: # docs:end array_is_scalar_example
   :dedent: 4

``isContiguous`` and ``isContiguousInStyleC``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Array.isContiguous

.. automethod:: Array.isContiguousInStyleC

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_is_contiguous_example
   :end-before: # docs:end array_is_contiguous_example
   :dedent: 4

``isContiguousInStyleFortran``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automethod:: Array.isContiguousInStyleFortran

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_is_contiguous_fortran_example
   :end-before: # docs:end array_is_contiguous_fortran_example
   :dedent: 4

``hasString``
~~~~~~~~~~~~~

.. automethod:: Array.hasString

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_has_string_example
   :end-before: # docs:end array_has_string_example
   :dedent: 4

``extractString``
~~~~~~~~~~~~~~~~~

.. automethod:: Array.extractString

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_extract_string_example
   :end-before: # docs:end array_extract_string_example
   :dedent: 4

``getFlatIndex``
~~~~~~~~~~~~~~~~

.. automethod:: Array.getFlatIndex

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_get_flat_index_example
   :end-before: # docs:end array_get_flat_index_example
   :dedent: 4

``getPrintString``
~~~~~~~~~~~~~~~~~~

.. automethod:: Array.getPrintString

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_get_print_string_example
   :end-before: # docs:end array_get_print_string_example
   :dedent: 4

``print``
~~~~~~~~~

.. automethod:: Array.print

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_print_example
   :end-before: # docs:end array_print_example
   :dedent: 4

``info``
~~~~~~~~

.. automethod:: Array.info

.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :start-after: # docs:start array_info_example
   :end-before: # docs:end array_info_example
   :dedent: 4
