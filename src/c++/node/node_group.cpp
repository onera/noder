# include "node/node_group.hpp"
# include "node/node.hpp"

# include <stdexcept>
# include <string>
# include <unordered_set>

namespace {

void ensureNodeNotNull(const std::shared_ptr<Node>& node, const char* context) {
    if (!node) {
        throw std::invalid_argument(std::string(context) + ": null node is not allowed");
    }
}

bool isAlreadyAttachedTo(const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent) {
    auto current_parent = node->parent().lock();
    return current_parent && current_parent.get() == parent.get();
}

std::shared_ptr<Node> canonicalNodeForPlus(const std::shared_ptr<Node>& node) {
    ensureNodeNotNull(node, "operator+");
    auto representative = node->expressionRepresentative();
    if (representative) {
        return representative;
    }
    return node;
}

std::vector<std::shared_ptr<Node>> mergeUniqueNodes(
    const std::vector<std::shared_ptr<Node>>& lhs,
    const std::vector<std::shared_ptr<Node>>& rhs) {

    std::vector<std::shared_ptr<Node>> merged;
    merged.reserve(lhs.size() + rhs.size());

    std::unordered_set<const Node*> seen;
    seen.reserve(lhs.size() + rhs.size());

    auto appendUnique = [&](const std::vector<std::shared_ptr<Node>>& nodes) {
        for (const auto& node : nodes) {
            auto canonicalNode = canonicalNodeForPlus(node);
            if (seen.insert(canonicalNode.get()).second) {
                merged.push_back(canonicalNode);
            }
        }
    };

    appendUnique(lhs);
    appendUnique(rhs);
    return merged;
}

std::shared_ptr<Node> firstResolvedParent(const std::vector<std::shared_ptr<Node>>& nodes) {
    for (const auto& node : nodes) {
        auto parent = node->parent().lock();
        if (parent) {
            return parent;
        }
    }
    return nullptr;
}

void attachAllToParent(
    const std::vector<std::shared_ptr<Node>>& nodes,
    const std::shared_ptr<Node>& parent) {

    for (const auto& node : nodes) {
        if (!isAlreadyAttachedTo(node, parent)) {
            node->attachTo(parent);
        }
    }
}

NodeGroup makeNodeGroup(std::vector<std::shared_ptr<Node>> nodes) {
    auto parent = firstResolvedParent(nodes);
    if (parent) {
        attachAllToParent(nodes, parent);
    }
    return NodeGroup(std::move(nodes));
}

} // namespace

NodeGroup::NodeGroup(std::vector<std::shared_ptr<Node>> nodes) : _nodes(std::move(nodes)) {}

const std::vector<std::shared_ptr<Node>>& NodeGroup::nodes() const {
    return _nodes;
}

bool NodeGroup::empty() const {
    return _nodes.empty();
}

void Node::setExpressionRepresentative(std::shared_ptr<Node> node) {
    _expressionRepresentative = node;
}

std::shared_ptr<Node> Node::expressionRepresentative() const {
    return _expressionRepresentative.lock();
}

NodeGroup operator+(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) {
    ensureNodeNotNull(lhs, "operator+");
    ensureNodeNotNull(rhs, "operator+");
    return makeNodeGroup(mergeUniqueNodes({lhs}, {rhs}));
}

NodeGroup operator+(NodeGroup lhs, const std::shared_ptr<Node>& rhs) {
    ensureNodeNotNull(rhs, "operator+");
    return makeNodeGroup(mergeUniqueNodes(lhs.nodes(), {rhs}));
}

NodeGroup operator+(const std::shared_ptr<Node>& lhs, NodeGroup rhs) {
    ensureNodeNotNull(lhs, "operator+");
    return makeNodeGroup(mergeUniqueNodes({lhs}, rhs.nodes()));
}

NodeGroup operator+(NodeGroup lhs, NodeGroup rhs) {
    return makeNodeGroup(mergeUniqueNodes(lhs.nodes(), rhs.nodes()));
}

std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) {
    ensureNodeNotNull(lhs, "operator/");
    ensureNodeNotNull(rhs, "operator/");
    if (!isAlreadyAttachedTo(rhs, lhs)) {
        rhs->attachTo(lhs);
    }
    rhs->setExpressionRepresentative(lhs);
    return rhs;
}

std::shared_ptr<Node> operator/(const std::shared_ptr<Node>& lhs, const NodeGroup& rhs) {
    ensureNodeNotNull(lhs, "operator/");

    if (rhs.empty()) {
        throw std::invalid_argument("operator/: node group is empty");
    }

    for (const auto& child : rhs.nodes()) {
        ensureNodeNotNull(child, "operator/");
        if (!isAlreadyAttachedTo(child, lhs)) {
            child->attachTo(lhs);
        }
    }

    auto returnedNode = rhs.nodes().back();
    returnedNode->setExpressionRepresentative(lhs);
    return returnedNode;
}
