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

    std::shared_ptr<Node> byName(const std::string& name, const size_t& depth=100);

    std::shared_ptr<Node> byNameRegex(const std::string& name_pattern, const size_t& depth=100);

    std::shared_ptr<Node> childByType(const std::string& type);

    std::shared_ptr<Node> byType(const std::string& type, const size_t& depth=100);

    std::shared_ptr<Node> byTypeRegex(const std::string& name_pattern, const size_t& depth=100);

    std::shared_ptr<Node> childByData(const std::string& data);

    std::shared_ptr<Node> childByData(const char* data);

    template <typename T>
    std::shared_ptr<Node> childByData(const T& data);
    
    std::shared_ptr<Node> byData(const std::string& data, const size_t& depth=100);

    std::shared_ptr<Node> byData(const char* data, const size_t& depth=100);

    template <typename T>
    std::shared_ptr<Node> byData(const T& data, const size_t& depth=100);


};

# endif 
