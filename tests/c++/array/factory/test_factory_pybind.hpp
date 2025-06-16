#ifndef TEST_ARRAY_FACTORY_PYBIND_HPP
#define TEST_ARRAY_FACTORY_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_c_to_py_pybind.hpp"
# include "test_strings_pybind.hpp"
# include "test_vectors_pybind.hpp"
# include "test_matrices_pybind.hpp"

void bindTestsOfFactoryOfArrays(py::module_ &m) {
    py::module_ factory = m.def_submodule("factory", "Array factory tests submodule");

    bindTestsOfFactoryOfArraysFromCToPy(factory);

    bindTestsOfFactoryOfArraysFromStrings(factory);

    bindTestsOfFactoryOfVectorArrays(factory);

    bindTestsOfFactoryOfMatricesArrays(factory);

}

#endif // TEST_ARRAY_FACTORY_PYBIND_HPP
