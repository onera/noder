# ifndef TEST_NAVIGATION_PYBIND_HPP
# define TEST_NAVIGATION_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_navigation.hpp"

void bindTestsOfNavigation(py::module_ &m) {

    py::module_ sm = m.def_submodule("navigation");
    
    sm.def("childByName", &test_childByName);
    sm.def("byName", &test_byName);
    sm.def("byNamePattern", &test_byNamePattern);
    
    sm.def("childByType", &test_childByType);
    sm.def("byType", &test_byType);
    sm.def("byTypePattern", &test_byTypePattern);
    
    sm.def("childByData", &test_childByData);
    sm.def("childByDataUsingChar", &test_childByDataUsingChar);
    sm.def("childByDataScalarDirect", &test_childByDataScalarDirect);

    sm.def("byData", &test_byData);
}

# endif
