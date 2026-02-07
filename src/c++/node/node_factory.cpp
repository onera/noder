# include "node/node_factory.hpp"


template <typename T>
std::shared_ptr<Node> newNode(
    const std::string& name,
    const std::string& type,
    const T& data,
    std::shared_ptr<Node> parent) {
    
    std::shared_ptr<Node> node = std::make_shared<Node>();
    node->setName(name);
    node->setType(type);
    node->setData(Array(data));
    if (parent) node->attachTo(parent);
    
    return node;
}

/*
    template instantiations
*/

// template <typename... T>
// struct Instantiator {
//     template <typename... U>
//     void operator()() const {
//         (static_cast<void>(newNode<U>(std::string{},std::string{},T{},std::shared_ptr<Node>{})), ...);
//     }
// };


template <typename... Ts>
struct Instantiator {
    template <typename... Us>
    void operator()() const {
        (utils::forceSymbol(&newNode<Us>), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::StringAndScalarTypes>();

// explicit instantiation when data is Array required for pybind
template std::shared_ptr<Node> newNode<Array>(
    const std::string&, const std::string&, const Array&, std::shared_ptr<Node>);
