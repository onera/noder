# ifndef TEST_NAVIGATION_PYBIND_HPP
# define TEST_NAVIGATION_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_navigation.hpp"

void bindTestsOfNavigation(py::module_ &m) {

    py::module_ sm = m.def_submodule("navigation");
    
    sm.def("childByName", &test_childByName);
    sm.def("byName", &test_byName);
    sm.def("allByName", &test_allByName);
    sm.def("byNameRegex", &test_byNameRegex);
    sm.def("allByNameRegex", &test_allByNameRegex);
    sm.def("byNameGlob", &test_byNameGlob);
    sm.def("allByNameGlob", &test_allByNameGlob);
    
    sm.def("childByType", &test_childByType);
    sm.def("byType", &test_byType);
    sm.def("allByType", &test_allByType);
    sm.def("byTypeRegex", &test_byTypeRegex);
    sm.def("allByTypeRegex", &test_allByTypeRegex);
    sm.def("byTypeGlob", &test_byTypeGlob);
    sm.def("allByTypeGlob", &test_allByTypeGlob);
    
    sm.def("childByData", &test_childByData);
    sm.def("childByDataUsingChar", &test_childByDataUsingChar);
    sm.def("childByDataScalarDirect", &test_childByDataScalarDirect);

    sm.def("byData", &test_byData);
    sm.def("allByData", &test_allByData);
    sm.def("byDataGlob", &test_byDataGlob);
    sm.def("allByDataGlob", &test_allByDataGlob);
    sm.def("byDataScalarDirect", &test_byDataScalarDirect);
    sm.def("allByDataScalarDirect", &test_allByDataScalarDirect);

    sm.def("byAnd", &test_byAnd);
    sm.def("byAndScalar", &test_byAndScalar);
    sm.def("allByAnd", &test_allByAnd);
}

# endif
