# ifndef FACTORY_PYBIND_HPP
# define FACTORY_PYBIND_HPP

# include "strings_pybind.hpp"
# include "vectors_pybind.hpp"
# include "matrices_pybind.hpp"

void bindFactoryOfArrays(py::module_ &m) {
    py::module_ factory = m.def_submodule("factory", "Array factory submodule");

    bindFactoryOfArraysFromStrings(factory);

    bindFactoryOfVectorArrays(factory);

    bindFactoryOfMatricesArrays(factory);

}

# endif
