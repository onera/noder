#ifndef IO_HDF5_CGNS_CGNS_IO_HPP
#define IO_HDF5_CGNS_CGNS_IO_HPP

#ifdef ENABLE_HDF5_IO

#include "node/node.hpp"

#include <memory>
#include <string>

namespace io::hdf5::cgns {

void write_node(const std::string& filename, std::shared_ptr<Node> node, const float& cgnsVersion = 3.1f);
std::shared_ptr<Node> read(const std::string& filename);

} // namespace io::hdf5::cgns

#endif // ENABLE_HDF5_IO

#endif // IO_HDF5_CGNS_CGNS_IO_HPP
