# ifndef ARRAY_FACTORY_VECTORS_PYBIND_HPP
# define ARRAY_FACTORY_VECTORS_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/vectors.hpp"

using namespace arrayfactory;

void bindFactoryOfVectorArrays(py::module_ &m) {

    utils::bindForFloatingAndIntegralTypes(m,
        "uniformFromStep", []<typename T>() {
            return &uniformFromStep<T>;
    });

    utils::bindForFloatingTypes(m,
        "uniformFromCount", []<typename T>() {
            return &uniformFromCount<T>;
    });
}

# endif
