#include "io/hdf5/cgns/node_pycgns_converter.hpp"

#include <unordered_map>

#include "array/array_numpy_bridge.hpp"

namespace {

struct AliasedPyCGNSNode {
    std::weak_ptr<Node> node;
    py::list pyList;
};

using AliasedPyCGNSRegistry = std::unordered_map<const Node*, AliasedPyCGNSNode>;

AliasedPyCGNSRegistry& aliasedPyCGNSRegistry() {
    static auto* registry = new AliasedPyCGNSRegistry();
    return *registry;
}

void removeExpiredAliasedNodes() {
    auto& registry = aliasedPyCGNSRegistry();
    for (auto it = registry.begin(); it != registry.end(); ) {
        if (it->second.node.expired()) {
            it = registry.erase(it);
        } else {
            ++it;
        }
    }
}

void rememberAliasedPyCGNS(const std::shared_ptr<Node>& node, const py::list& pyList) {
    if (!node) {
        return;
    }

    removeExpiredAliasedNodes();
    aliasedPyCGNSRegistry()[node.get()] = AliasedPyCGNSNode{node, pyList};
}

py::object aliasedPyCGNSOrNone(const std::shared_ptr<Node>& node) {
    if (!node) {
        return py::none();
    }

    removeExpiredAliasedNodes();

    const auto it = aliasedPyCGNSRegistry().find(node.get());
    if (it == aliasedPyCGNSRegistry().end()) {
        return py::none();
    }

    if (it->second.node.expired()) {
        aliasedPyCGNSRegistry().erase(it);
        return py::none();
    }

    return py::reinterpret_borrow<py::object>(it->second.pyList);
}

void validatePyCGNSNodeList(const py::list& pyList) {
    if (!py::isinstance<py::list>(pyList) || py::len(pyList) != 4) {
        throw std::invalid_argument("pyCGNSToNode: Input must be a 4-element Python list.");
    }
}

py::object nodeValueToPyCGNS(const std::shared_ptr<Node>& node) {
    if (node->hasLinkTarget()) {
        py::list linkValue;
        linkValue.append("target_file:" + node->linkTargetFile());
        linkValue.append("target_path:" + node->linkTargetPath());
        return std::move(linkValue);
    }

    std::shared_ptr<Data> dataPtr = node->dataPtr();
    if (!dataPtr || dataPtr->isNone()) {
        return py::none();
    }

    if (auto arrayPtr = std::dynamic_pointer_cast<Array>(dataPtr)) {
        return arraybridge::toPyObject(*arrayPtr);
    }

    throw std::runtime_error("nodeToPyCGNS: Unsupported Data subclass.");
}

py::list buildPyCGNSSnapshot(const std::shared_ptr<Node>& node);
py::list buildRegisteredPyCGNS(const std::shared_ptr<Node>& node);
void synchronizeAliasedPyCGNSRecursive(const std::shared_ptr<Node>& node);

py::list getOrCreateAliasedPyCGNS(const std::shared_ptr<Node>& node) {
    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (aliasedPyObject.is_none()) {
        return buildRegisteredPyCGNS(node);
    }

    synchronizeAliasedPyCGNSRecursive(node);
    return aliasedPyObject.cast<py::list>();
}

py::list buildPyCGNSNodeList(const std::shared_ptr<Node>& node, bool registerAlias) {
    py::list nodePyList;
    py::list childrenPyList;

    nodePyList.append(node->name());
    nodePyList.append(nodeValueToPyCGNS(node));
    nodePyList.append(childrenPyList);
    nodePyList.append(node->type());

    if (registerAlias) {
        rememberAliasedPyCGNS(node, nodePyList);
    }

    for (const auto& child : node->children()) {
        if (registerAlias) {
            childrenPyList.append(buildRegisteredPyCGNS(child));
        } else {
            childrenPyList.append(buildPyCGNSSnapshot(child));
        }
    }

    return nodePyList;
}

py::list buildPyCGNSSnapshot(const std::shared_ptr<Node>& node) {
    return buildPyCGNSNodeList(node, false);
}

py::list buildRegisteredPyCGNS(const std::shared_ptr<Node>& node) {
    return buildPyCGNSNodeList(node, true);
}

py::list mutableChildrenList(const py::list& nodePyList) {
    if (py::isinstance<py::list>(nodePyList[2])) {
        return py::cast<py::list>(nodePyList[2]);
    }

    py::list childrenPyList;
    nodePyList[2] = childrenPyList;
    return childrenPyList;
}

void synchronizeAliasedPyCGNSRecursive(const std::shared_ptr<Node>& node) {
    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (aliasedPyObject.is_none()) {
        return;
    }

    py::list nodePyList = aliasedPyObject.cast<py::list>();
    nodePyList[0] = py::cast(node->name());
    nodePyList[1] = nodeValueToPyCGNS(node);

    py::list childrenPyList = mutableChildrenList(nodePyList);
    childrenPyList.attr("clear")();
    for (const auto& child : node->children()) {
        childrenPyList.append(getOrCreateAliasedPyCGNS(child));
    }

    nodePyList[3] = py::cast(node->type());
}

void populateNodeValueFromPyCGNS(const std::shared_ptr<Node>& node, const py::object& valueObj) {
    if (py::isinstance<py::array>(valueObj)) {
        node->setData(arraybridge::arrayFromPyArray(py::cast<py::array>(valueObj)));
        return;
    }

    if (valueObj.is_none()) {
        return;
    }

    if (py::isinstance<py::list>(valueObj)) {
        py::list linkValue = py::cast<py::list>(valueObj);
        std::string targetFile;
        std::string targetPath;

        for (const auto& item : linkValue) {
            if (!py::isinstance<py::str>(item)) {
                continue;
            }

            const std::string token = py::cast<std::string>(item);
            if (token.rfind("target_file:", 0) == 0) {
                targetFile = token.substr(std::string("target_file:").size());
            } else if (token.rfind("target_path:", 0) == 0) {
                targetPath = token.substr(std::string("target_path:").size());
            }
        }

        if (targetPath.empty()) {
            throw std::invalid_argument("pyCGNSToNode: Link value list must define target_path.");
        }

        node->setLinkTarget(targetFile, targetPath);
        return;
    }

    throw std::invalid_argument(
        "pyCGNSToNode: Second element must be NumPy array, None, or Link value list.");
}

std::shared_ptr<Node> pyCGNSToNodeRecursive(const py::list& pyList) {
    validatePyCGNSNodeList(pyList);

    const std::string name = py::cast<std::string>(pyList[0]);
    const std::string type = py::cast<std::string>(pyList[3]);

    std::shared_ptr<Node> node = std::make_shared<Node>(name, type);
    rememberAliasedPyCGNS(node, pyList);

    populateNodeValueFromPyCGNS(node, pyList[1].cast<py::object>());

    if (!py::isinstance<py::list>(pyList[2])) {
        throw std::invalid_argument("pyCGNSToNode: Third element must be a list.");
    }

    py::list children = py::cast<py::list>(pyList[2]);
    for (const auto& child : children) {
        if (!py::isinstance<py::list>(child)) {
            throw std::invalid_argument("pyCGNSToNode: Child must be a list.");
        }

        node->addChild(pyCGNSToNodeRecursive(py::cast<py::list>(child)));
    }

    return node;
}

} // namespace

namespace pycgnsinterop {

void synchronizeAliasedPyCGNSIfPresent(const std::shared_ptr<Node>& node) {
    if (!node) {
        return;
    }

    synchronizeAliasedPyCGNSRecursive(node);
}

} // namespace pycgnsinterop

/**
 * @brief Convert a C++ Node object into a Python CGNS-like structure (list).
 * @param node The Node to convert.
 * @return A pybind11 list representing the Node in CGNS format.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node) {
    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (!aliasedPyObject.is_none()) {
        synchronizeAliasedPyCGNSRecursive(node);
        return aliasedPyObject.cast<py::list>();
    }

    return buildPyCGNSSnapshot(node);
}

/**
 * @brief Convert a Python CGNS-like list into a C++ Node object.
 * @param pyList A 4-element list in CGNS format.
 * @return A shared_ptr<Node> representing the converted Node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList) {
    return pyCGNSToNodeRecursive(pyList);
}
