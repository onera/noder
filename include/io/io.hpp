# ifdef ENABLE_HDF5_IO
# ifndef IO_HPP
# define IO_HPP

#include "node/node.hpp"

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

} // namespace io

# endif // IO_HPP
# endif // ENABLE_HDF5_IO
