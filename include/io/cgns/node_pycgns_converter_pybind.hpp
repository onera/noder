# ifndef NODE_PYCGNS_CONVERTER_PYBIND_HPP
# define NODE_PYCGNS_CONVERTER_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "io/cgns/node_pycgns_converter.hpp"

void bindNodePyCGNSConverter(py::module_ &m) {
    m.def("nodeToPyCGNS", &nodeToPyCGNS, "Convert a Node to a Python CGNS-like list.");
    m.def("pyCGNSToNode", &pyCGNSToNode, "Convert a Python CGNS-like list to a Node.");
}

# endif