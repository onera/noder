# ifdef ENABLE_HDF5_IO
# ifndef IO_HPP
# define IO_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include "node/node.hpp"

namespace py = pybind11;

namespace io {

void write_node(const std::string& filename, std::shared_ptr<Node> node);
std::shared_ptr<Node> read(const std::string& filename);

void write_numpy(const py::array& array, const std::string& filename, const std::string& dataset_name);
py::array read_numpy(const std::string& filename, const std::string& dataset_name, const std::string& order="F");

} // namespace io

# endif // IO_HPP
# endif // ENABLE_HDF5_IO
