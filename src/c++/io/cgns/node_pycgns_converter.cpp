# include "io/cgns/node_pycgns_converter.hpp"

/**
 * @brief Convert a C++ Node object into a Python CGNS-like structure (list).
 * @param node The Node to convert.
 * @return A pybind11 list representing the Node in CGNS format.
 */
py::list nodeToPyCGNS(const std::shared_ptr<Node>& node) {
    py::list nodePyList;

    // Append node name
    nodePyList.append(node->name());

    // Convert node data (value)
    std::shared_ptr<Data> dataPtr = node->dataPtr();
    if (!dataPtr || dataPtr->isNone()) {
        nodePyList.append(py::none());
    } else if (auto arrayPtr = std::dynamic_pointer_cast<Array>(dataPtr)) {
        nodePyList.append(arrayPtr->getPyArray());
    } else {
        throw std::runtime_error("nodeToPyCGNS: Unsupported Data subclass.");
    }

    // Convert children to Python list
    py::list children_pylist;
    for (const auto& child : node->children()) {
        children_pylist.append(nodeToPyCGNS(child));
    }
    nodePyList.append(children_pylist);

    // Append node type
    nodePyList.append(node->type());

    return nodePyList;
}

/**
 * @brief Convert a Python CGNS-like list into a C++ Node object.
 * @param pyList A 4-element list in CGNS format.
 * @return A shared_ptr<Node> representing the converted Node.
 */
std::shared_ptr<Node> pyCGNSToNode(const py::list& pyList) {
    if (!py::isinstance<py::list>(pyList) || py::len(pyList) != 4) {
        throw std::invalid_argument("pyCGNSToNode: Input must be a 4-element Python list.");
    }

    // Extract name
    std::string name = py::cast<std::string>(pyList[0]);

    // Extract value (Data)
    std::shared_ptr<Data> dataPtr;
    if (py::isinstance<py::array>(pyList[1])) {
        dataPtr = std::make_shared<Array>(py::cast<py::array>(pyList[1]));
    } else if (py::isinstance<py::none>(pyList[1])) {
        dataPtr = nullptr;  // No value
    } else {
        throw std::invalid_argument("pyCGNSToNode: Second element must be a NumPy array or None.");
    }

    // Extract children
    std::shared_ptr<Node> node = std::make_shared<Node>(name);
    node->setData(std::move(dataPtr));

    if (!py::isinstance<py::list>(pyList[2])) {
        throw std::invalid_argument("pyCGNSToNode: Third element must be a list.");
    }

    py::list children = py::cast<py::list>(pyList[2]);
    for (const auto& child : children) {
        if (py::isinstance<py::list>(child)) {
            node->addChild(pyCGNSToNode(py::cast<py::list>(child)));  // Recursively create child nodes
        } else {
            throw std::invalid_argument("pyCGNSToNode: Child must be a list.");
        }
    }

    // Extract type
    node->setType(py::cast<std::string>(pyList[3]));

    return node;
}

/**
 * @brief Bind the conversion functions to Pybind11.
 */
PYBIND11_MODULE(noder_core, m) {
    m.def("nodeToPyCGNS", &nodeToPyCGNS, "Convert a Node to a Python CGNS-like list.");
    m.def("pyCGNSToNode", &pyCGNSToNode, "Convert a Python CGNS-like list to a Node.");
}
