# ifndef ARRAY_FACTORY_VECTORS_PYBIND_HPP
# define ARRAY_FACTORY_VECTORS_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/vectors.hpp"

using namespace arrayfactory;

void bindFactoryOfVectorArrays(py::module_ &m) {

    utils::bindForFloatingAndIntegralTypes(m, "uniformFromStep", 
        []<typename T>(utils::TypeTag<T>) { return &uniformFromStep<T>; },
        py::arg("start"),
        py::arg("stop"),
        py::arg("step") = 1.0,
        R"doc(
Build a uniformly-spaced 1D Array from ``start`` to ``stop`` (excluded) with ``step``.

Returns
-------
Array
    New one-dimensional numeric array.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_vectors.py
   :language: python
   :pyobject: test_uniformFromStep_positive_step
)doc"
    );

    utils::bindForFloatingTypes(m, "uniformFromCount", 
        []<typename T>(utils::TypeTag<T>) { return &uniformFromCount<T>; },
        py::arg("start"),
        py::arg("stop"),
        py::arg("num"),
        py::arg("endpoint") = true,
        R"doc(
Build a uniformly-spaced 1D Array with ``num`` points between ``start`` and ``stop``.

Returns
-------
Array
    New one-dimensional numeric array.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_vectors.py
   :language: python
   :pyobject: test_uniformFromCount_true
)doc"
    );
}

# endif
