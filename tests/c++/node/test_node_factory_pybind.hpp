# ifndef TEST_NODE_FACTORY_PYBIND_HPP
# define TEST_NODE_FACTORY_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_node_factory.hpp"

void bindTestsOfNodeFactory(py::module_ &m) {

    py::module_ sm = m.def_submodule("node_factory");
    
    sm.def("test_newNodeNoArgs", &test_newNodeNoArgs);
    sm.def("test_newNodeNoArgsHasNoneData", &test_newNodeNoArgsHasNoneData);
    sm.def("test_newNodeOnlyName", &test_newNodeOnlyName);
    sm.def("test_newNodeOnlyNameChars", &test_newNodeOnlyNameChars);
    sm.def("test_newNodeNameAndType", &test_newNodeNameAndType);
    sm.def("test_newNodeNameTypeAndData", &test_newNodeNameTypeAndData);
    sm.def("test_newNodeDataString", &test_newNodeDataString);
    sm.def("test_newNodeDataStringChars", &test_newNodeDataStringChars);
    sm.def("test_newNodeParent", &test_newNodeParent);
}

# endif
