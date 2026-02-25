Utils (Python)
==============

.. currentmodule:: noder.array.data_types

Module
------

.. automodule:: noder.array.data_types
   :members:
   :undoc-members:

Purpose
-------
Provide dtype catalogs and argument validators used by Python-side factory
helpers.

Arguments
---------
Validator helpers accept ``dtype`` or ``order`` strings.

Output
------
Validator helpers return ``None`` and raise exceptions on invalid inputs.

Example
-------

.. literalinclude:: ../../../tests/python/array/test_data_types.py
   :language: python
   :pyobject: test_raise_error_if_invalid_order_accepts_known_order
