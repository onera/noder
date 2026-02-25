Array Factory (Python)
======================

.. currentmodule:: noder.core.factory

Module
------

.. automodule:: noder.core.factory
   :members:
   :undoc-members:

Purpose
-------
The ``factory`` module creates typed arrays (vectors, matrices, and strings).
Generated function names are suffixed by dtype, for example ``zeros_float``
or ``uniformFromStep_int32``.

Arguments
---------
Depend on each function family:

- vector generation: ``start``, ``stop``, ``step`` or ``num``.
- matrix generation: ``shape``, ``order`` and optional fill value.
- string conversion: input text.

Output
------
``Array`` instances.

Example
-------

.. literalinclude:: ../../../tests/python/array/factory/test_vectors.py
   :language: python
   :pyobject: test_uniformFromStep_positive_step

.. literalinclude:: ../../../tests/python/array/factory/test_matrices.py
   :language: python
   :pyobject: test_zeros

.. literalinclude:: ../../../tests/python/array/factory/test_strings.py
   :language: python
   :pyobject: test_arrayFromString
