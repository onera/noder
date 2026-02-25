.. _cpp-arrayfactory-module:

Array Factory (C++)
===================

Factory helpers for creating ``Array`` instances from numeric, matrix, and string inputs.

Namespace reference: :doc:`generated/namespace_arrayfactory`

Header references:

- :doc:`generated/file_include_array_factory_vectors.hpp`
- :doc:`generated/file_include_array_factory_matrices.hpp`
- :doc:`generated/file_include_array_factory_strings.hpp`
- :doc:`generated/file_include_array_factory_c_to_py.hpp`

Examples
--------

.. literalinclude:: ../../../tests/c++/array/factory/test_vectors.cpp
   :language: cpp
   :start-after: void test_uniformFromCount_positive_step() {
   :end-before: template <typename T>

.. literalinclude:: ../../../tests/c++/array/factory/test_matrices.cpp
   :language: cpp
   :start-after: void test_zeros_c_order() {
   :end-before: template <typename T>
