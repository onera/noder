#ifndef NODE_EXPANSION_HPP
#define NODE_EXPANSION_HPP

#include <cstddef>
#include <memory>

class Node;

/**
 * @brief Loading state of a node's direct children.
 */
enum class ChildrenLoadState {
    Unloaded,
    Partial,
    Complete
};

/**
 * @brief Backend-neutral hook for lazily expanding a Node.
 *
 * The interface deliberately contains no HDF5 or CGNS types.  Ordinary
 * in-memory Nodes do not install an expansion backend.  A lazy reader may
 * install an implementation that materializes metadata children in batches
 * and loads payloads only when requested.
 */
class NodeExpansion {
public:
    virtual ~NodeExpansion() = default;

    /** @brief Return the current direct-child loading state. */
    virtual ChildrenLoadState childrenLoadState(const Node& node) const = 0;

    /**
     * @brief Ensure at least the requested number of children are materialized.
     *
     * A request of ``std::numeric_limits<std::size_t>::max()`` means all
     * children.  Implementations may materialize more than the requested
     * number when that is more efficient.
     */
    virtual void ensureChildrenLoaded(Node& node, std::size_t minimumChildren) = 0;

    /** @brief Load the complete payload of a node when it is needed. */
    virtual void ensureDataLoaded(Node& node) = 0;

    /** @brief Inspect whether a payload exists without loading it. */
    virtual bool hasData(const Node& node) const = 0;

    /**
     * @brief Notify the backend that a payload was assigned through Node's
     * regular mutation API.
     *
     * Backends may use this to mark their payload cache as resolved.  The
     * default implementation is sufficient for read-only/eager adapters.
     */
    virtual void dataAssigned(Node& /*node*/) {}
};

#endif // NODE_EXPANSION_HPP
