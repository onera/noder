# ifndef TEST_ARRAY_FACTORY_STRINGS_PYBIND_HPP
# define TEST_ARRAY_FACTORY_STRINGS_PYBIND_HPP

# include "test_strings.hpp"

void bindTestsOfFactoryOfArraysFromStrings(py::module_ &m) {

    m.def("arrayFromString", &test_arrayFromString);

    m.def("arrayFromUnicodeString", &test_arrayFromUnicodeString);

}

# endif