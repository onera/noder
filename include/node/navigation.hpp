# ifndef NAVIGATION_HPP
# define NAVIGATION_HPP

# include <string>
# include <memory>
# include <regex>

# include "utils/template_instantiator.hpp"


class Node;

class Navigation {

private:
    Node& _node;

public:

    explicit Navigation(Node& ownerNode);

    std::shared_ptr<Node> childByName(const std::string& name);

    std::shared_ptr<Node> byName(const std::string& name, const int& depth=100);

    std::shared_ptr<Node> byNamePattern(const std::string& name_pattern, const int& depth=100);

    std::shared_ptr<Node> childByType(const std::string& type);

    std::shared_ptr<Node> byType(const std::string& type, const int& depth=100);

    std::shared_ptr<Node> byTypePattern(const std::string& name_pattern, const int& depth=100);

    std::shared_ptr<Node> childByData(const std::string& data);

    std::shared_ptr<Node> childByData(const char* data);

    template <typename T>
    std::shared_ptr<Node> childByData(T data);
    
    std::shared_ptr<Node> byData(const std::string& data, const int& depth=100);

    std::shared_ptr<Node> byData(const char* data, const int& depth=100);


};

# endif 
