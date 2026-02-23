#ifndef NODE_FACTORY_HPP
#define NODE_FACTORY_HPP

# include "node/node.hpp"
# include "array/array.hpp"

template <typename T> 
std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const T& data,
    std::shared_ptr<Node> parent = nullptr);


// Overload: default data = bool(false)
inline std::shared_ptr<Node> newNode(
    const std::string& name = "",
    const std::string& type = "",
    std::shared_ptr<Node> parent = nullptr)
{
    return newNode<Array>(name, type, Array(), parent);
}

inline std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const char* data,
    std::shared_ptr<Node> parent = nullptr)
{
    return newNode<std::string>(name, type, std::string(data), parent);
}

#endif
