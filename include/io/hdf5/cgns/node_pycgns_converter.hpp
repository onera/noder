#ifndef IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP
#define IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "array/array.hpp"
#include "node/node.hpp"

namespace py = pybind11;

/**
 * @brief Convert a Node hierarchy to a fresh Python CGNS-like nested list.
 * The nested list structure is copied, while compatible NumPy payload buffers
 * remain shared with the Node hierarchy.
 * @param node Root node to convert.
 * @return Nested list representation.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node, bool strictTypes = true);

/**
 * @brief Convert a Python CGNS-like nested list to a Node hierarchy.
 * The input list structure is not retained or modified. Compatible NumPy
 * payload buffers remain shared with the converted Node hierarchy.
 * @param pyList Nested list representation.
 * @return Converted root node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList, bool strictTypes = true);

#endif // IO_HDF5_CGNS_NODE_PYCGNS_CONVERTER_HPP
