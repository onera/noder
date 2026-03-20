#ifndef IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP
#define IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "array/array.hpp"
#include "node/node.hpp"

namespace py = pybind11;

namespace pycgnsinterop {

/**
 * @brief Synchronize the aliased Python CGNS list attached to a Node, when one exists.
 * @param node Node whose aliased pyCGNS view should be refreshed.
 */
void synchronizeAliasedPyCGNSIfPresent(const std::shared_ptr<Node>& node);

} // namespace pycgnsinterop

/**
 * @brief Convert a Node hierarchy to a Python CGNS-like nested list.
 * @param node Root node to convert.
 * @return Nested list representation.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node);

/**
 * @brief Convert a Python CGNS-like nested list to a Node hierarchy.
 * @param pyList Nested list representation.
 * @return Converted root node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList);

#endif // IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP
