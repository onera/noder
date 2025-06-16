# ifndef NAVIGATION_HPP
# define NAVIGATION_HPP

# include <string>
# include <memory>
# include <regex>

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

};

# endif 
