# ifdef ENABLE_HDF5_IO
# ifndef IO_HPP
# define IO_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include "node/node.hpp"

namespace py = pybind11;

namespace io {

/**
 * @brief Write a Node hierarchy to disk in CGNS/HDF5 layout.
 * @param filename Output file path.
 * @param node Root node to serialize.
 * @param cgnsVersion CGNS version metadata.
 */
void write_node(const std::string& filename, std::shared_ptr<Node> node, const float& cgnsVersion = 3.1);
/**
 * @brief Read a Node hierarchy from disk.
 * @param filename Input file path.
 * @return Root node of the loaded hierarchy.
 */
std::shared_ptr<Node> read(const std::string& filename);

/**
 * @brief Persist a NumPy array into an HDF5 dataset.
 * @param array Data to write.
 * @param filename Output file path.
 * @param dataset_name Dataset key.
 */
void write_numpy(const py::array& array, const std::string& filename, const std::string& dataset_name);
/**
 * @brief Load a NumPy array from an HDF5 dataset.
 * @param filename Input file path.
 * @param dataset_name Dataset key.
 * @param order Memory order of returned array ("C" or "F").
 * @return Loaded NumPy array.
 */
py::array read_numpy(const std::string& filename, const std::string& dataset_name, const std::string& order="F");

} // namespace io

# endif // IO_HPP
# endif // ENABLE_HDF5_IO
