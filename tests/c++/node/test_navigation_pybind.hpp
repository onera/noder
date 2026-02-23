# ifndef TEST_NAVIGATION_PYBIND_HPP
# define TEST_NAVIGATION_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_navigation.hpp"

void bindTestsOfNavigation(py::module_ &m) {

    py::module_ sm = m.def_submodule("navigation");
    
    sm.def("childByName", &test_childByName);
    sm.def("byName", &test_byName);
    sm.def("byNameRegex", &test_byNameRegex);
    
    sm.def("childByType", &test_childByType);
    sm.def("byType", &test_byType);
    sm.def("byTypeRegex", &test_byTypeRegex);
    
    sm.def("childByData", &test_childByData);
    sm.def("childByDataUsingChar", &test_childByDataUsingChar);
    sm.def("childByDataScalarDirect", &test_childByDataScalarDirect);

    sm.def("byData", &test_byData);
    sm.def("byDataScalarDirect", &test_byDataScalarDirect);
}

# endif
