# ifndef NODE_GROUP_PYBIND_HPP
# define NODE_GROUP_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "node/node_group.hpp"
# include "node/node.hpp"

void bindNodeGroup(py::module_ &m, py::class_<Node, std::shared_ptr<Node>>& nodeClass) {
    py::class_<NodeGroup>(m, "_NodeGroup")
        .def("__add__", [](const NodeGroup& lhs, std::shared_ptr<Node> rhs) {
            return lhs + rhs;
        }, py::is_operator())
        .def("__add__", [](const NodeGroup& lhs, const NodeGroup& rhs) {
            return lhs + rhs;
        }, py::is_operator())
        .def("nodes", &NodeGroup::nodes)
    ;

    nodeClass
        .def("__add__", [](std::shared_ptr<Node> lhs, std::shared_ptr<Node> rhs) {
            return lhs + rhs;
        }, py::is_operator())
        .def("__add__", [](std::shared_ptr<Node> lhs, const NodeGroup& rhs) {
            return lhs + rhs;
        }, py::is_operator())
        .def("__truediv__", [](std::shared_ptr<Node> lhs, std::shared_ptr<Node> rhs) {
            return lhs / rhs;
        }, py::is_operator())
        .def("__truediv__", [](std::shared_ptr<Node> lhs, const NodeGroup& rhs) {
            return lhs / rhs;
        }, py::is_operator())
    ;
}

# endif
