#ifndef NODE_FACTORY_HPP
#define NODE_FACTORY_HPP

# include "node/node.hpp"
# include "array/array.hpp"

template <typename T> [[gnu::used]]
std::shared_ptr<Node> newNode(
    const std::string& name = "",
    const std::string& type = "",
    const T& data = false,
    std::shared_ptr<Node> parent = nullptr);

#endif
