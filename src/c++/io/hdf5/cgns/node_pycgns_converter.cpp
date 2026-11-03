#include "io/hdf5/cgns/node_pycgns_converter.hpp"

#include <unordered_map>

#include "array/array_numpy_bridge.hpp"
#include "cgns/base.hpp"
#include "cgns/tree.hpp"
#include "cgns/zone.hpp"

namespace {

struct AliasedPyCGNSNode {
    std::weak_ptr<Node> node;
    py::list pyList;
    std::uint64_t revision;
    bool strictTypes;
    PyObject* nameObject;
    PyObject* valueObject;
    PyObject* childrenObject;
    PyObject* typeObject;
    Py_ssize_t childrenSize;
    std::uint64_t childrenFingerprint;
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

void periodicallyRemoveExpiredAliasedNodes() {
    // Sweeping the whole registry for every node makes a single conversion
    // quadratic.  Bound stale aliases while keeping the sweep cost amortized
    // across top-level conversions.
    static std::size_t conversionsSinceSweep = 0;
    constexpr std::size_t sweepInterval = 128;
    if (++conversionsSinceSweep == sweepInterval) {
        removeExpiredAliasedNodes();
        conversionsSinceSweep = 0;
    }
}

std::uint64_t fingerprintChildren(const py::list& children) {
    std::uint64_t fingerprint = 1469598103934665603ULL;
    const Py_ssize_t size = PyList_GET_SIZE(children.ptr());
    for (Py_ssize_t index = 0; index < size; ++index) {
        fingerprint ^= reinterpret_cast<std::uintptr_t>(
            PyList_GET_ITEM(children.ptr(), index));
        fingerprint *= 1099511628211ULL;
    }
    return fingerprint;
}

void saveAliasedPyCGNSState(AliasedPyCGNSNode& entry, const std::shared_ptr<Node>& node) {
    PyObject* list = entry.pyList.ptr();
    PyObject* children = PyList_GET_ITEM(list, 2);
    entry.revision = node->revision();
    entry.nameObject = PyList_GET_ITEM(list, 0);
    entry.valueObject = PyList_GET_ITEM(list, 1);
    entry.childrenObject = children;
    entry.typeObject = PyList_GET_ITEM(list, 3);
    entry.childrenSize = PyList_GET_SIZE(children);
    entry.childrenFingerprint = fingerprintChildren(
        py::reinterpret_borrow<py::list>(children));
}

bool aliasedPyCGNSIsCurrent(
    const AliasedPyCGNSNode& entry,
    const std::shared_ptr<Node>& node,
    const bool strictTypes) {
    if (entry.revision != node->revision() || entry.strictTypes != strictTypes) {
        return false;
    }

    PyObject* list = entry.pyList.ptr();
    if (PyList_GET_SIZE(list) != 4 ||
        PyList_GET_ITEM(list, 0) != entry.nameObject ||
        PyList_GET_ITEM(list, 1) != entry.valueObject ||
        PyList_GET_ITEM(list, 2) != entry.childrenObject ||
        PyList_GET_ITEM(list, 3) != entry.typeObject ||
        !PyList_Check(entry.childrenObject) ||
        PyList_GET_SIZE(entry.childrenObject) != entry.childrenSize) {
        return false;
    }

    return fingerprintChildren(py::reinterpret_borrow<py::list>(entry.childrenObject)) ==
        entry.childrenFingerprint;
}

void rememberAliasedPyCGNS(
    const std::shared_ptr<Node>& node,
    const py::list& pyList,
    const bool strictTypes) {
    if (!node) {
        return;
    }

    auto& entry = aliasedPyCGNSRegistry()[node.get()];
    entry.node = node;
    entry.pyList = pyList;
    entry.strictTypes = strictTypes;
    saveAliasedPyCGNSState(entry, node);
}

py::object aliasedPyCGNSOrNone(const std::shared_ptr<Node>& node) {
    if (!node) {
        return py::none();
    }

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

py::object restrictPyCGNSType(py::object value) {
    if (!py::isinstance<py::array>(value)) {
        return value;
    }
    const py::dtype dtype = value.cast<py::array>().dtype();
    const char kind = dtype.kind();
    const auto itemSize = dtype.itemsize();
    const bool native = dtype.attr("isnative").cast<bool>();
    if (native &&
        ((kind == 'b' && itemSize == 1) ||
         (kind == 'i' && itemSize < static_cast<ssize_t>(sizeof(int32_t))))) {
        return value.attr("astype")(py::dtype::of<int32_t>());
    }
    if (native &&
        kind == 'u' && itemSize < static_cast<ssize_t>(sizeof(uint32_t))) {
        return value.attr("astype")(py::dtype::of<uint32_t>());
    }
    return value;
}

py::object nodeValueToPyCGNS(const std::shared_ptr<Node>& node, const bool strictTypes) {
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
        if (arrayPtr->hasString()) {
            // PyCGNS and Cassiopee expect CGNS C1 payloads as one byte per
            // array element. Legacy fixed-width NumPy strings (for example,
            // shape (1,), dtype S10) are otherwise written as just their
            // first character by Cassiopee.
            const std::string value = arrayPtr->extractString();
            py::module_ numpy = py::module_::import("numpy");
            return numpy.attr("frombuffer")(
                py::bytes(value), numpy.attr("dtype")("S1")).attr("copy")();
        }
        py::object value = arraybridge::toPyObject(*arrayPtr);
        return strictTypes ? restrictPyCGNSType(std::move(value)) : value;
    }

    throw std::runtime_error("nodeToPyCGNS: Unsupported Data subclass.");
}

py::list buildPyCGNSSnapshot(const std::shared_ptr<Node>& node, bool strictTypes);
py::list buildRegisteredPyCGNS(const std::shared_ptr<Node>& node, bool strictTypes);
void synchronizeAliasedPyCGNSRecursive(const std::shared_ptr<Node>& node, bool strictTypes);

py::list getOrCreateAliasedPyCGNS(const std::shared_ptr<Node>& node, const bool strictTypes) {
    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (aliasedPyObject.is_none()) {
        return buildRegisteredPyCGNS(node, strictTypes);
    }

    const auto it = aliasedPyCGNSRegistry().find(node.get());
    if (it == aliasedPyCGNSRegistry().end() ||
        !aliasedPyCGNSIsCurrent(it->second, node, strictTypes)) {
        synchronizeAliasedPyCGNSRecursive(node, strictTypes);
    }
    return aliasedPyObject.cast<py::list>();
}

py::list buildPyCGNSNodeList(const std::shared_ptr<Node>& node, bool registerAlias, const bool strictTypes) {
    py::list nodePyList;
    py::list childrenPyList;

    nodePyList.append(node->name());
    nodePyList.append(nodeValueToPyCGNS(node, strictTypes));
    nodePyList.append(childrenPyList);
    nodePyList.append(node->type());

    for (const auto& child : node->children()) {
        if (registerAlias) {
            childrenPyList.append(buildRegisteredPyCGNS(child, strictTypes));
        } else {
            childrenPyList.append(buildPyCGNSSnapshot(child, strictTypes));
        }
    }

    if (registerAlias) {
        rememberAliasedPyCGNS(node, nodePyList, strictTypes);
    }

    return nodePyList;
}

py::list buildPyCGNSSnapshot(const std::shared_ptr<Node>& node, const bool strictTypes) {
    return buildPyCGNSNodeList(node, false, strictTypes);
}

py::list buildRegisteredPyCGNS(const std::shared_ptr<Node>& node, const bool strictTypes) {
    return buildPyCGNSNodeList(node, true, strictTypes);
}

py::list mutableChildrenList(const py::list& nodePyList) {
    if (py::isinstance<py::list>(nodePyList[2])) {
        return py::cast<py::list>(nodePyList[2]);
    }

    py::list childrenPyList;
    nodePyList[2] = childrenPyList;
    return childrenPyList;
}

void synchronizeAliasedPyCGNSRecursive(const std::shared_ptr<Node>& node, const bool strictTypes) {
    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (aliasedPyObject.is_none()) {
        return;
    }

    const auto current = aliasedPyCGNSRegistry().find(node.get());
    if (current == aliasedPyCGNSRegistry().end()) {
        return;
    }
    if (aliasedPyCGNSIsCurrent(current->second, node, strictTypes)) {
        return;
    }

    py::list nodePyList = aliasedPyObject.cast<py::list>();
    py::object name = py::cast(node->name());
    py::object value = nodeValueToPyCGNS(node, strictTypes);
    py::object type = py::cast(node->type());
    if (PyList_GET_SIZE(nodePyList.ptr()) != 4) {
        nodePyList.attr("clear")();
        nodePyList.append(name);
        nodePyList.append(value);
        nodePyList.append(py::list());
        nodePyList.append(type);
    } else {
        nodePyList[0] = name;
        nodePyList[1] = value;
        nodePyList[3] = type;
    }

    py::list childrenPyList = mutableChildrenList(nodePyList);
    childrenPyList.attr("clear")();
    for (const auto& child : node->children()) {
        childrenPyList.append(getOrCreateAliasedPyCGNS(child, strictTypes));
    }

    // Creating a missing alias for a newly attached child may rehash the
    // registry, so reacquire the iterator after synchronizing descendants.
    const auto updated = aliasedPyCGNSRegistry().find(node.get());
    if (updated != aliasedPyCGNSRegistry().end()) {
        updated->second.strictTypes = strictTypes;
        saveAliasedPyCGNSState(updated->second, node);
    }
}

void populateNodeValueFromPyCGNS(
    const std::shared_ptr<Node>& node,
    const py::list& pyList,
    const py::object& valueObj,
    const bool strictTypes) {
    if (py::isinstance<py::array>(valueObj)) {
        py::object source = strictTypes ? restrictPyCGNSType(valueObj) : valueObj;
        if (!source.is(valueObj)) {
            pyList[1] = source;
        }
        auto arrayData = std::make_shared<Array>(
            arraybridge::arrayFromPyArray(py::cast<py::array>(source)));
        node->setData(arrayData);

        // Only string and None-like arrays need payload normalization here.
        // Numeric arrays already alias the source NumPy object; visiting all
        // of them a second time erased much of the cost saved by caching.
        if (arrayData->hasString() || arrayData->isNone()) {
            py::object normalizedValue = nodeValueToPyCGNS(node, strictTypes);
            if (!normalizedValue.is(pyList[1])) {
                pyList[1] = std::move(normalizedValue);
            }
        }
    } else if (valueObj.is_none()) {
        return;
    } else if (py::isinstance<py::list>(valueObj)) {
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
        py::object normalizedValue = nodeValueToPyCGNS(node, strictTypes);
        if (!normalizedValue.is(valueObj)) {
            pyList[1] = std::move(normalizedValue);
        }
    } else {
        throw std::invalid_argument(
            "pyCGNSToNode: Second element must be NumPy array, None, or Link value list.");
    }

}

std::shared_ptr<Node> makeNodeForCGNSType(
    const std::string& name,
    const std::string& type) {

    if (type == "Zone_t") {
        return std::make_shared<Zone>(name);
    }
    if (type == "CGNSBase_t") {
        return std::make_shared<Base>(name);
    }
    if (type == "CGNSTree_t" || type == "Root Node of HDF5 File") {
        auto tree = std::make_shared<Tree>();
        tree->setName(name);
        tree->setType(type);
        return tree;
    }

    return std::make_shared<Node>(name, type);
}

std::shared_ptr<Node> pyCGNSToNodeRecursive(const py::list& pyList, const bool strictTypes) {
    validatePyCGNSNodeList(pyList);

    const std::string name = py::cast<std::string>(pyList[0]);
    const std::string type = py::cast<std::string>(pyList[3]);

    std::shared_ptr<Node> node = makeNodeForCGNSType(name, type);
    populateNodeValueFromPyCGNS(node, pyList, pyList[1].cast<py::object>(), strictTypes);

    if (!py::isinstance<py::list>(pyList[2])) {
        throw std::invalid_argument("pyCGNSToNode: Third element must be a list.");
    }

    py::list children = py::cast<py::list>(pyList[2]);
    std::vector<std::shared_ptr<Node>> childNodes;
    childNodes.reserve(static_cast<size_t>(py::len(children)));
    for (const auto& child : children) {
        if (!py::isinstance<py::list>(child)) {
            throw std::invalid_argument("pyCGNSToNode: Child must be a list.");
        }

        childNodes.push_back(pyCGNSToNodeRecursive(py::cast<py::list>(child), strictTypes));
    }
    node->addChildren(childNodes);

    // Node::addChild historically resolves duplicate sibling names by keeping
    // the last node.  Reflect that policy in the aliased list once, so the
    // revision cache can return the same Python subtree without rescanning it.
    const auto& attachedChildren = node->children();
    if (attachedChildren.size() != static_cast<size_t>(py::len(children))) {
        children.attr("clear")();
        for (const auto& child : attachedChildren) {
            children.append(aliasedPyCGNSOrNone(child));
        }
    }

    rememberAliasedPyCGNS(node, pyList, strictTypes);

    return node;
}

} // namespace

namespace pycgnsinterop {

void synchronizeAliasedPyCGNSIfPresent(const std::shared_ptr<Node>& node) {
    if (!node) {
        return;
    }

    synchronizeAliasedPyCGNSRecursive(node, true);
}

} // namespace pycgnsinterop

/**
 * @brief Convert a C++ Node object into a Python CGNS-like structure (list).
 * @param node The Node to convert.
 * @return A pybind11 list representing the Node in CGNS format.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node, const bool strictTypes) {
    periodicallyRemoveExpiredAliasedNodes();

    py::object aliasedPyObject = aliasedPyCGNSOrNone(node);
    if (!aliasedPyObject.is_none()) {
        synchronizeAliasedPyCGNSRecursive(node, strictTypes);
        return aliasedPyObject.cast<py::list>();
    }

    return buildPyCGNSSnapshot(node, strictTypes);
}

/**
 * @brief Convert a Python CGNS-like list into a C++ Node object.
 * @param pyList A 4-element list in CGNS format.
 * @return A shared_ptr<Node> representing the converted Node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList, const bool strictTypes) {
    periodicallyRemoveExpiredAliasedNodes();
    return pyCGNSToNodeRecursive(pyList, strictTypes);
}
