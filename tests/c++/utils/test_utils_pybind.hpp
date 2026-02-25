# ifndef TEST_UTILS_PYBIND_HPP
# define TEST_UTILS_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_utils.hpp"

void bindTestsOfUtils(py::module_ &m) {
    py::module_ sm = m.def_submodule("utils");

    sm.def("test_stringStartsWith", &test_stringStartsWith);
    sm.def("test_stringEndsWith", &test_stringEndsWith);
    sm.def("test_clipStringIfTooLong", &test_clipStringIfTooLong);
    sm.def("test_approxEqual", &test_approxEqual);
}

# endif
