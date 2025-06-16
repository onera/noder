# ifndef NODE_PYCGNS_CONVERTER_HPP
# define NODE_PYCGNS_CONVERTER_HPP

# include <pybind11/pybind11.h>
# include <pybind11/stl.h>
# include <pybind11/numpy.h>

# include "node/node.hpp"
# include "array/array.hpp"  

namespace py = pybind11;

py::list nodeToPyCGNS(const std::shared_ptr<Node>& node);

std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList);

# endif