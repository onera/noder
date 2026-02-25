# ifndef NODE_GROUP_PYBIND_HPP
# define NODE_GROUP_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "node/node_group.hpp"
# include "node/node.hpp"

void bindNodeGroup(py::module_ &m, py::class_<Node, std::shared_ptr<Node>>& nodeClass) {
    py::class_<NodeGroup>(
        m,
        "_NodeGroup",
        R"doc(
Internal helper type returned by Node expression operators.

This type groups nodes for chained expressions such as ``a / (b + c)``.
)doc")
        .def("__add__", [](const NodeGroup& lhs, std::shared_ptr<Node> rhs) {
            return lhs + rhs;
        }, py::is_operator(), R"doc(
Append a node to this group and return a new group.
)doc")
        .def("__add__", [](const NodeGroup& lhs, const NodeGroup& rhs) {
            return lhs + rhs;
        }, py::is_operator(), R"doc(
Concatenate two node groups.
)doc")
        .def("nodes", &NodeGroup::nodes, R"doc(
Return grouped nodes in insertion order.

Returns
-------
list[Node]
    Grouped nodes.
)doc")
    ;

    nodeClass
        .def("__add__", [](std::shared_ptr<Node> lhs, std::shared_ptr<Node> rhs) {
            return lhs + rhs;
        }, py::is_operator(), R"doc(
Create a :py:class:`_NodeGroup` from two nodes.

Example
-------
.. literalinclude:: ../../../tests/python/node/test_node_group.py
   :language: python
   :pyobject: test_operator_plus_attach_to_first_parent
)doc")
        .def("__add__", [](std::shared_ptr<Node> lhs, const NodeGroup& rhs) {
            return lhs + rhs;
        }, py::is_operator(), R"doc(
Prefix a node before an existing :py:class:`_NodeGroup`.
)doc")
        .def("__truediv__", [](std::shared_ptr<Node> lhs, std::shared_ptr<Node> rhs) {
            return lhs / rhs;
        }, py::is_operator(), R"doc(
Attach ``rhs`` as child of ``lhs`` and return ``rhs``.
)doc")
        .def("__truediv__", [](std::shared_ptr<Node> lhs, const NodeGroup& rhs) {
            return lhs / rhs;
        }, py::is_operator(), R"doc(
Attach all nodes in ``rhs`` as children of ``lhs`` and return the last node.
)doc")
    ;
}

# endif
