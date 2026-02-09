# ifndef ARRAY_FACTORY_MATRICES_PYBIND_HPP
# define ARRAY_FACTORY_MATRICES_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/matrices.hpp"


using namespace arrayfactory;


void bindFactoryOfMatricesArrays(py::module_ &m) {

    utils::bindForScalarTypes(m, "full", 
        []<typename T>(utils::TypeTag<T>) { return &full<T>; }
    );

    utils::bindForScalarTypes(m, "empty", 
        []<typename T>(utils::TypeTag<T>) { return &empty<T>; }
    );

    utils::bindForScalarTypes(m, "computeStridesInOrderC",
        []<typename T>(utils::TypeTag<T>) { return &computeStridesInOrderC<T>; }
    );
    utils::bindForScalarTypes(m, "computeStridesInOrderF",
        []<typename T>(utils::TypeTag<T>) { return &computeStridesInOrderF<T>; }
    );

    utils::bindForScalarTypes(m, "zeros",
        []<typename T>(utils::TypeTag<T>) { return &zeros<T>; }
    );

    utils::bindForScalarTypes(m, "ones",
        []<typename T>(utils::TypeTag<T>) { return &ones<T>; }
    );

}

# endif
