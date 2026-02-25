# ifndef NODE_GROUP_HPP
# define NODE_GROUP_HPP

// visibility parameters for exporting symbols, since Node is used as a library
# ifndef NODE_EXPORT
    #if defined(_WIN32) || defined(__CYGWIN__)
        #ifdef BUILDING_NODE_LIBRARY
            #define NODE_EXPORT __declspec(dllexport)
        #else
            #define NODE_EXPORT __declspec(dllimport)
        #endif
    #elif __GNUC__ >= 4
        #define NODE_EXPORT __attribute__((visibility("default")))
    #else
        #define NODE_EXPORT
    #endif
# endif

# include <memory>
# include <vector>

class Node;

/**
 * @brief Temporary grouping object used by Node expression operators.
 *
 * A NodeGroup preserves insertion order and is mainly produced by `operator+`
 * to support chained expressions such as `a / (b + c + d)`.
 */
class NODE_EXPORT NodeGroup {
private:
    std::vector<std::shared_ptr<Node>> _nodes;

public:
    /** @brief Construct a group from an ordered node sequence. */
    explicit NodeGroup(std::vector<std::shared_ptr<Node>> nodes = {});

    /** @brief Access grouped nodes in insertion order. */
    const std::vector<std::shared_ptr<Node>>& nodes() const;
    /** @brief True when no nodes are stored. */
    bool empty() const;
};

/** @brief Build a group from two nodes. */
NODE_EXPORT NodeGroup operator+(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs);
/** @brief Append a node to an existing group. */
NODE_EXPORT NodeGroup operator+(NodeGroup lhs, const std::shared_ptr<Node>& rhs);
/** @brief Prefix a node before an existing group. */
NODE_EXPORT NodeGroup operator+(const std::shared_ptr<Node>& lhs, NodeGroup rhs);
/** @brief Concatenate two groups. */
NODE_EXPORT NodeGroup operator+(NodeGroup lhs, NodeGroup rhs);

/** @brief Attach rhs under lhs and return rhs. */
NODE_EXPORT std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs);
/** @brief Attach each node in rhs group under lhs and return the last one. */
NODE_EXPORT std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const NodeGroup& rhs);

# endif
