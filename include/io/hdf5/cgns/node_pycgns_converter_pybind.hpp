#ifndef IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP
#define IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP

#include <pybind11/pybind11.h>

#include "cgns/zone.hpp"
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
strict_types : bool, optional
    Promote narrow integer and boolean payloads to CGNS-compatible types. Defaults to ``True``.

Returns
-------
list
    CGNS-like list representation.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_nodeToPyCGNS_tree
)doc",
        py::arg("arg0"), py::arg("strict_types") = true);
    m.def(
        "pyCGNSToNode",
        [](const py::list& pyList, const bool strictTypes) {
            py::module_::import("noder._cgns");
            auto node = pyCGNSToNode(pyList, strictTypes);
            if (auto zone = std::dynamic_pointer_cast<Zone>(node)) {
                return py::cast(zone);
            }
            return py::cast(node);
        },
        R"doc(
Convert a Python CGNS-like nested list into a Node hierarchy.

Parameters
----------
arg0 : list
    CGNS-like list representation.
strict_types : bool, optional
    Promote narrow integer and boolean payloads to CGNS-compatible types. Defaults to ``True``.

Returns
-------
Node
    Converted root node.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_pyCGNSToNode_tree
)doc",
        py::arg("arg0"), py::arg("strict_types") = true);
}

#endif // IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_PYBIND_HPP
