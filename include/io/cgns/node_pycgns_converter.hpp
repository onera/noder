# ifndef NODE_PYCGNS_CONVERTER_HPP
# define NODE_PYCGNS_CONVERTER_HPP

# include <pybind11/pybind11.h>
# include <pybind11/stl.h>
# include <pybind11/numpy.h>

# include "node/node.hpp"
# include "array/array.hpp"  

namespace py = pybind11;

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

# endif
