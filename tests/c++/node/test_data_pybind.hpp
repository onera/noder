# ifndef TEST_DATA_PYBIND_HPP
# define TEST_DATA_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_data.hpp"


void bindTestsOfData(py::module_ &m) {

    py::module_ sm = m.def_submodule("data");
    
    sm.def("hasString", &testdata::test_hasString);
    sm.def("isNone", &testdata::test_isNone);
    sm.def("isScalar", &testdata::test_isScalar);
    sm.def("expectedString", &testdata::test_expectedString);
    sm.def("extractString", &testdata::test_extractString);
    sm.def("isEqualToInteger", &testdata::test_isEqualToInteger);
}

# endif
