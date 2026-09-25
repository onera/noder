#ifndef IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_HPP
#define IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_HPP

#ifdef ENABLE_HDF5_IO

#include "node/node.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace io::hdf5::cgns {

/**
 * @brief Lazy metadata/payload reader for CGNS files stored in HDF5.
 *
 * The reader keeps the HDF5 file open and creates metadata-only Node objects.
 * Direct children are materialized on demand, optionally in batches, and
 * payloads are read only when Node data is requested.
 */
class NODE_EXPORT LazyHdf5Reader {
public:
    explicit LazyHdf5Reader(const std::string& filename, char order = 'F');
    ~LazyHdf5Reader();

    LazyHdf5Reader(const LazyHdf5Reader&) = delete;
    LazyHdf5Reader& operator=(const LazyHdf5Reader&) = delete;
    LazyHdf5Reader(LazyHdf5Reader&&) noexcept;
    LazyHdf5Reader& operator=(LazyHdf5Reader&&) noexcept;

    /** @brief Return the lazily-backed root node. */
    std::shared_ptr<Node> root() const;

    /** @brief Source filename. */
    const std::string& filename() const;

    /** @brief Normalized payload memory order ('C' or 'F'). */
    char order() const;

    /** @brief Close the underlying HDF5 file explicitly. */
    void close() const;

    /** @brief Whether the underlying HDF5 file is still open. */
    bool isOpen() const;

    /** @brief Explicitly request a direct-child page for a node. */
    void ensureChildrenLoaded(
        const std::shared_ptr<Node>& node,
        std::size_t minimumChildren = 0) const;

    /** @brief Explicitly request full payload loading for a node. */
    void ensureDataLoaded(const std::shared_ptr<Node>& node) const;

private:
    class State;
    std::shared_ptr<State> _state;
    std::shared_ptr<Node> _root;
};

} // namespace io::hdf5::cgns

#endif // ENABLE_HDF5_IO

#endif // IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_HPP
