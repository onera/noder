#include "node/navigation.hpp"
#include "node/node.hpp"

Navigation::Navigation(Node& inputNode) : _node(inputNode) {}

std::shared_ptr<Node> Navigation::childByName(const std::string& name) {

    for (auto child: _node.children()) {
        if (child && name == child->name()) return child;
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byName(const std::string& name, const int& depth) {

    if ( depth > 0 ) {
        if ( _node.name() == name ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byName(name, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byNamePattern(const std::string& namePattern, const int& depth) {

    if ( depth > 0 ) {
        std::regex regexPattern(namePattern);
        if (std::regex_search(_node.name(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byNamePattern(namePattern, depth-1);
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


std::shared_ptr<Node> Navigation::byType(const std::string& type, const int& depth) {

    if ( depth > 0 ) {
        if ( _node.type() == type ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byType(type, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byTypePattern(const std::string& typePattern,
                                                const int& depth) {
    if ( depth > 0 ) {
        std::regex regexPattern(typePattern);
        if (std::regex_search(_node.type(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byTypePattern(typePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}
