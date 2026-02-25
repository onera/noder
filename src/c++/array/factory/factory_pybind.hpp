# ifndef FACTORY_PYBIND_HPP
# define FACTORY_PYBIND_HPP

# include "strings_pybind.hpp"
# include "vectors_pybind.hpp"
# include "matrices_pybind.hpp"

void bindFactoryOfArrays(py::module_ &m) {
    py::module_ factory = m.def_submodule(
        "factory",
        R"doc(
Typed array-construction helpers.

Functions are generated per dtype suffix (for example ``zeros_float`` or
``uniformFromStep_int32``).

See C++ counterpart: :ref:`cpp-arrayfactory-module`.
)doc");

    bindFactoryOfArraysFromStrings(factory);

    bindFactoryOfVectorArrays(factory);

    bindFactoryOfMatricesArrays(factory);

}

# endif
