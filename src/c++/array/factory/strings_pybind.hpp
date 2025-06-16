# ifndef ARRAY_FACTORY_STRINGS_PYBIND_HPP
# define ARRAY_FACTORY_STRINGS_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/strings.hpp"

using namespace arrayfactory;

void bindFactoryOfArraysFromStrings(py::module_ &m) {

    m.def("arrayFromString", &arrayFromString, py::arg("string"), "Convert string to Array");

    m.def("arrayFromUnicodeString", &arrayFromUnicodeString, py::arg("string"), "Convert Unicode string to Array");

}

# endif // ARRAY_FACTORY_STRINGS_PYBIND_HPP
