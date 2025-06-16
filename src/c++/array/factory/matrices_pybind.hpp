# ifndef ARRAY_FACTORY_MATRICES_PYBIND_HPP
# define ARRAY_FACTORY_MATRICES_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/matrices.hpp"


using namespace arrayfactory;


void bindFactoryOfMatricesArrays(py::module_ &m) {

    utils::bindForScalarTypes(m, "full", []<typename T>() { return &full<T>; });

    utils::bindForScalarTypes(m, "empty", []<typename T>() { return &empty<T>; });

    utils::bindForScalarTypes(m, "computeStridesInOrderC", []<typename T>() { return &computeStridesInOrderC<T>; });
    utils::bindForScalarTypes(m, "computeStridesInOrderF", []<typename T>() { return &computeStridesInOrderF<T>; });

    utils::bindForScalarTypes(m, "zeros", []<typename T>() { return &zeros<T>; });    

    utils::bindForScalarTypes(m, "ones", []<typename T>() { return &ones<T>; });    

}

# endif
