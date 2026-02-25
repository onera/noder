#ifndef NODE_FACTORY_HPP
#define NODE_FACTORY_HPP

# include "node/node.hpp"
# include "array/array.hpp"

/**
 * @brief Construct a Node with typed payload and optional parent.
 *
 * @tparam T Payload source type accepted by Node::setData.
 * @param name Node name.
 * @param type Node type string.
 * @param data Payload value.
 * @param parent Optional parent for immediate attachment.
 */
template <typename T> 
std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const T& data,
    std::shared_ptr<Node> parent = nullptr);


/** @brief Construct a Node with default empty Array payload. */
inline std::shared_ptr<Node> newNode(
    const std::string& name = "",
    const std::string& type = "",
    std::shared_ptr<Node> parent = nullptr)
{
    return newNode<Array>(name, type, Array(), parent);
}

/** @brief C-string payload convenience overload. */
inline std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const char* data,
    std::shared_ptr<Node> parent = nullptr)
{
    return newNode<std::string>(name, type, std::string(data), parent);
}

#endif
