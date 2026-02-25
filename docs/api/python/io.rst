IO (Python)
===========

.. currentmodule:: noder.core.io

Module
------

.. automodule:: noder.core.io
   :members:

Purpose
-------
Read and write node trees and NumPy arrays using HDF5-backed storage.

Arguments
---------
File paths, dataset names, and data payloads.

Output
------
``Node`` trees or ``numpy.ndarray`` values.

Example
-------

.. literalinclude:: ../../../tests/python/io/test_io.py
   :language: python
   :pyobject: test_write_and_read_numerical_numpy
