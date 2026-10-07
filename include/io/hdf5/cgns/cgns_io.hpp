#ifndef IO_HDF5_CGNS_CGNS_IO_HPP
#define IO_HDF5_CGNS_CGNS_IO_HPP

#ifdef ENABLE_HDF5_IO

#include "node/node.hpp"

#include <memory>
#include <string>

namespace io::hdf5::cgns {

void write_node(const std::string& filename, std::shared_ptr<Node> node, const float& cgnsVersion = 3.1f);

/**
 * @brief Update one existing CGNS/HDF5 node without reading or rewriting the file tree.
 *
 * The path is relative to the persisted HDF5 root (the ``CGNSTree_t`` wrapper
 * is omitted for flattened CGNS/HDF5 files).  The operation updates the node
 * metadata and payload in place.  If the node does not exist, the node subtree
 * is appended below its existing parent.
 */
void write_node_only(
    const std::string& filename,
    std::shared_ptr<Node> node,
    const std::string& persistedPath,
    const std::string& persistedLinkTargetPath = std::string());

std::shared_ptr<Node> read(
    const std::string& filename,
    const char order = 'F',
    bool safeMode = false);

std::shared_ptr<Node> read(const std::string& filename, bool safeMode);

} // namespace io::hdf5::cgns

#endif // ENABLE_HDF5_IO

#endif // IO_HDF5_CGNS_CGNS_IO_HPP
