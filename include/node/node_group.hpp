# ifndef NODE_GROUP_HPP
# define NODE_GROUP_HPP

// visibility parameters for exporting symbols, since Node is used as a library
# ifndef NODE_EXPORT
    #if defined(_WIN32) || defined(__CYGWIN__)
        #ifdef BUILDING_NODE_LIBRARY
            #define NODE_EXPORT __declspec(dllexport)
        #else
            #define NODE_EXPORT __declspec(dllimport)
        #endif
    #elif __GNUC__ >= 4
        #define NODE_EXPORT __attribute__((visibility("default")))
    #else
        #define NODE_EXPORT
    #endif
# endif

# include <memory>
# include <vector>

class Node;

class NODE_EXPORT NodeGroup {
private:
    std::vector<std::shared_ptr<Node>> _nodes;

public:
    explicit NodeGroup(std::vector<std::shared_ptr<Node>> nodes = {});

    const std::vector<std::shared_ptr<Node>>& nodes() const;
    bool empty() const;
};

NODE_EXPORT NodeGroup operator+(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs);
NODE_EXPORT NodeGroup operator+(NodeGroup lhs, const std::shared_ptr<Node>& rhs);
NODE_EXPORT NodeGroup operator+(const std::shared_ptr<Node>& lhs, NodeGroup rhs);
NODE_EXPORT NodeGroup operator+(NodeGroup lhs, NodeGroup rhs);

NODE_EXPORT std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs);
NODE_EXPORT std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const NodeGroup& rhs);

# endif
