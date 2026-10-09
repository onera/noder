#ifndef NODE_EXPANSION_HPP
#define NODE_EXPANSION_HPP

#include <cstddef>
#include <memory>
#include <optional>

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

    /** @brief Return the number of materializable direct child nodes. */
    virtual std::size_t childCount(const Node& /*node*/) const { return 0; }

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
     * @brief Inspect whether the payload is scalar without loading its values.
     *
     * ``std::nullopt`` means that the backend cannot answer from metadata and
     * that the caller may resolve the payload if it needs the answer.
     */
    virtual std::optional<bool> dataIsScalar(const Node& /*node*/) const {
        return std::nullopt;
    }

    /** @brief Inspect whether a payload is string-like without loading it. */
    virtual std::optional<bool> dataIsString(const Node& /*node*/) const {
        return std::nullopt;
    }

    /** @brief Inspect whether the payload has already been materialized. */
    virtual bool dataIsLoaded(const Node& /*node*/) const { return true; }

    /**
     * @brief Notify the backend that a payload was assigned through Node's
     * regular mutation API.
     *
     * Backends may use this to mark their payload cache as resolved.  The
     * default implementation is sufficient for read-only/eager adapters.
     */
    virtual void dataAssigned(Node& /*node*/) {}

    /** @brief Discard a materialized payload while retaining lazy metadata. */
    virtual void dataUnloaded(Node& /*node*/) {}

    /**
     * @brief Temporarily release resources that prevent an external writer
     * from opening the source file for update.
     *
     * Lazy readers may keep a read-only HDF5 handle alive.  Targeted writers
     * call this hook around their update and then restore the reader state.
     */
    virtual void beginExternalWrite() {}

    /** @brief Reopen resources released by beginExternalWrite(). */
    virtual void endExternalWrite() {}
};

#endif // NODE_EXPANSION_HPP
