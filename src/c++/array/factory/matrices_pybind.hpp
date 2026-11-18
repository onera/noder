# ifndef ARRAY_FACTORY_MATRICES_PYBIND_HPP
# define ARRAY_FACTORY_MATRICES_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "utils/template_binder.hpp"
# include "array/factory/matrices.hpp"


using namespace arrayfactory;


void bindFactoryOfMatricesArrays(py::module_ &m) {

    utils::bindForScalarTypes(m, "full", 
        []<typename T>(utils::TypeTag<T>) { return &full<T>; },
        py::arg("shape"),
        py::arg("fill_value"),
        py::arg("order") = 'C',
        R"doc(
Allocate an Array of ``shape`` filled with ``fill_value``.

Returns
-------
Array
    Allocated numeric array.

)doc"
    );

    utils::bindForScalarTypes(m, "empty", 
        []<typename T>(utils::TypeTag<T>) { return &empty<T>; },
        py::arg("shape"),
        py::arg("order") = 'C',
        R"doc(
Allocate an uninitialized Array of ``shape``.

Returns
-------
Array
    Allocated array with unspecified initial content.

)doc"
    );

    utils::bindForScalarTypes(m, "computeStridesInOrderC",
        []<typename T>(utils::TypeTag<T>) { return &computeStridesInOrderC<T>; },
        py::arg("shape"),
        R"doc(
Compute C-order byte strides for a given ``shape``.

Returns
-------
list[int]
    Strides in bytes.
)doc"
    );
    utils::bindForScalarTypes(m, "computeStridesInOrderF",
        []<typename T>(utils::TypeTag<T>) { return &computeStridesInOrderF<T>; },
        py::arg("shape"),
        R"doc(
Compute Fortran-order byte strides for a given ``shape``.

Returns
-------
list[int]
    Strides in bytes.
)doc"
    );

    utils::bindForScalarTypes(m, "zeros",
        []<typename T>(utils::TypeTag<T>) { return &zeros<T>; }
        ,
        py::arg("shape"),
        py::arg("order") = 'C',
        R"doc(
Allocate a zero-initialized Array.

Returns
-------
Array
    Array filled with zeros.

)doc"
    );

    utils::bindForScalarTypes(m, "ones",
        []<typename T>(utils::TypeTag<T>) { return &ones<T>; }
        ,
        py::arg("shape"),
        py::arg("order") = 'C',
        R"doc(
Allocate a one-initialized Array.

Returns
-------
Array
    Array filled with ones.

)doc"
    );

}

# endif
