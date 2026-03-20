#ifndef IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP
#define IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP

#include <pybind11/pybind11.h>

#include "io/hdf5/cgns/node_pycgns_converter.hpp"

inline void bindNodePyCGNSConverter(py::module_& m) {
    m.def(
        "nodeToPyCGNS",
        &nodeToPyCGNS,
        R"doc(
Convert a Node hierarchy to a Python CGNS-like nested list.

Parameters
----------
arg0 : Node
    Root node to convert.

Returns
-------
list
    CGNS-like list representation.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_nodeToPyCGNS_tree
)doc");
    m.def(
        "pyCGNSToNode",
        &pyCGNSToNode,
        R"doc(
Convert a Python CGNS-like nested list into a Node hierarchy.

Parameters
----------
arg0 : list
    CGNS-like list representation.

Returns
-------
Node
    Converted root node.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_pyCGNSToNode_tree
)doc");
}

#endif // IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP
