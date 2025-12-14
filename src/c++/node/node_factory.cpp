# include "node/node_factory.hpp"


template <typename T> [[gnu::used]]
std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const T& data,
    std::shared_ptr<Node> parent) {
    
    std::shared_ptr<Node> node = std::make_shared<Node>();
    node->setName(name);
    node->setType(type);
    Array dataArray(data);
    node->setData(dataArray);
    if (parent) node->attachTo(parent);
    
    return node;
}

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(newNode<U>(std::string{},std::string{},T{},std::shared_ptr<Node>{})), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::StringAndScalarTypes>();
