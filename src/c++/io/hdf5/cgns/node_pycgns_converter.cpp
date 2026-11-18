#include "io/hdf5/cgns/node_pycgns_converter.hpp"

#include "array/array_numpy_bridge.hpp"
#include "cgns/base.hpp"
#include "cgns/tree.hpp"
#include "cgns/zone.hpp"

namespace {

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
        return linkValue;
    }

    std::shared_ptr<Data> dataPtr = node->dataPtr();
    if (!dataPtr) {
        return py::none();
    }

    if (auto arrayPtr = std::dynamic_pointer_cast<Array>(dataPtr)) {
        if (arrayPtr->typeId() == ArrayTypeId::None) {
            return py::none();
        }
        if (arrayPtr->hasString()) {
            if (arrayPtr->itemsize() == 1) {
                // For |S1 arrays the standard path preserves multi-dimensional
                // shape (e.g. (5,32) dtype |S1). Only fixed-width string
                // arrays need the extractString+frombuffer workaround.
                py::object value = arraybridge::toPyObject(*arrayPtr);
                return strictTypes ? restrictPyCGNSType(std::move(value)) : value;
            }
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

    if (dataPtr->isNone()) {
        return py::none();
    }

    throw std::runtime_error("nodeToPyCGNS: Unsupported Data subclass.");
}

py::list buildPyCGNSSnapshot(const std::shared_ptr<Node>& node, const bool strictTypes) {
    py::list nodePyList;
    py::list childrenPyList;

    nodePyList.append(node->name());
    nodePyList.append(nodeValueToPyCGNS(node, strictTypes));
    nodePyList.append(childrenPyList);
    nodePyList.append(node->type());

    for (const auto& child : node->children()) {
        childrenPyList.append(buildPyCGNSSnapshot(child, strictTypes));
    }

    return nodePyList;
}

void populateNodeValueFromPyCGNS(
    const std::shared_ptr<Node>& node,
    const py::object& valueObj,
    const bool strictTypes) {
    if (py::isinstance<py::array>(valueObj)) {
        py::object source = strictTypes ? restrictPyCGNSType(valueObj) : valueObj;
        auto arrayData = std::make_shared<Array>(
            arraybridge::arrayFromPyArray(py::cast<py::array>(source)));
        node->setData(arrayData);
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
    populateNodeValueFromPyCGNS(node, pyList[1].cast<py::object>(), strictTypes);

    if (!py::isinstance<py::list>(pyList[2])) {
        throw std::invalid_argument("pyCGNSToNode: Third element must be a list.");
    }

    py::list children = py::cast<py::list>(pyList[2]);
    std::vector<std::shared_ptr<Node>> childNodes;
    childNodes.reserve(py::len(children));
    for (const auto& child : children) {
        if (!py::isinstance<py::list>(child)) {
            throw std::invalid_argument("pyCGNSToNode: Child must be a list.");
        }

        childNodes.push_back(pyCGNSToNodeRecursive(py::cast<py::list>(child), strictTypes));
    }
    node->addChildren(childNodes);

    return node;
}

} // namespace

/**
 * @brief Convert a C++ Node object into a Python CGNS-like structure (list).
 * @param node The Node to convert.
 * @return A pybind11 list representing the Node in CGNS format.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node, const bool strictTypes) {
    return buildPyCGNSSnapshot(node, strictTypes);
}

/**
 * @brief Convert a Python CGNS-like list into a C++ Node object.
 * @param pyList A 4-element list in CGNS format.
 * @return A shared_ptr<Node> representing the converted Node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList, const bool strictTypes) {
    return pyCGNSToNodeRecursive(pyList, strictTypes);
}
