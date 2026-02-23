# ifndef TEST_NODE_GROUP_PYBIND_HPP
# define TEST_NODE_GROUP_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_node_group.hpp"

void bindTestsOfNodeGroup(py::module_ &m) {
    py::module_ sm = m.def_submodule("node_group");

    sm.def("test_operator_plus_attachToFirstParent", &test_operator_plus_attachToFirstParent);
    sm.def("test_operator_div_addChild_chain", &test_operator_div_addChild_chain);
    sm.def("test_operator_div_with_plus_group", &test_operator_div_with_plus_group);
    sm.def("test_operator_nested_expression_tree", &test_operator_nested_expression_tree);
}

# endif
