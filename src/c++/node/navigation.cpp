#include "node/navigation.hpp"
#include "node/node.hpp"

Navigation::Navigation(Node& inputNode) : _node(inputNode) {}

std::shared_ptr<Node> Navigation::childByName(const std::string& name) {

    for (auto child: _node.children()) {
        if (child && name == child->name()) return child;
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byName(const std::string& name, const size_t& depth) {

    if ( depth > 0 ) {
        if ( _node.name() == name ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byName(name, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byNameRegex(const std::string& namePattern, const size_t& depth) {

    if ( depth > 0 ) {
        std::regex regexPattern(namePattern);
        if (std::regex_search(_node.name(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byNameRegex(namePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::childByType(const std::string& type) {

    for (auto child: _node.children()) {
        if (child && type == child->type()) return child;
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byType(const std::string& type, const size_t& depth) {

    if ( depth > 0 ) {
        if ( _node.type() == type ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byType(type, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byTypeRegex(const std::string& typePattern,
                                                const size_t& depth) {
    if ( depth > 0 ) {
        std::regex regexPattern(typePattern);
        if (std::regex_search(_node.type(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byTypeRegex(typePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::childByData(const std::string& data) {

    for (auto child: _node.children()) {
        if (child) {

            bool isDataString = child->data().hasString();
            
            if (isDataString) {
                std::string dataAsString = child->data().extractString();

                if (data == dataAsString) return child;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::childByData(const char* data) {
    std::string data_str = std::string(data);
    return this->childByData(data_str);
}

template <typename T>
std::shared_ptr<Node> Navigation::childByData(const T& data) {

    for (auto child: _node.children()) {
        if (child) {

            bool isScalar = child->data().isScalar();
            
            if (isScalar) {
                if (child->data() == data) return child;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byData(const std::string& data, const size_t& depth) {

    if ( depth > 0 ) {

        bool thisHasDataString = _node.data().hasString();

        if (thisHasDataString) {
            if (_node.data().extractString() == data ) {
                return _node.shared_from_this();
            }
        }

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byData(data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byData(const char* data, const size_t& depth) {
    std::string data_str = std::string(data);
    return this->byData(data_str, depth);
}

template <typename T>
std::shared_ptr<Node> Navigation::byData(const T& data, const size_t& depth) {

    if ( depth > 0 ) {

        if (_node.data().isScalar()) {
            if (_node.data() == data ) {
                return _node.shared_from_this();
            }
        }

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byData(data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}



/*
    template instantiations
*/

template <typename... T>
struct InstantiatorMethodScalar {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&Navigation::template childByData<U>), ...);
        (utils::forceSymbol(&Navigation::template byData<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorMethodScalar, utils::ScalarTypes>();
template std::shared_ptr<Node> Navigation::childByData<bool>(const bool&);
template std::shared_ptr<Node> Navigation::childByData<double>(const double&);
template std::shared_ptr<Node> Navigation::childByData<int>(const int&);
template std::shared_ptr<Node> Navigation::byData<bool>(const bool&, const size_t&);
template std::shared_ptr<Node> Navigation::byData<double>(const double&, const size_t&);
template std::shared_ptr<Node> Navigation::byData<int>(const int&, const size_t&);
