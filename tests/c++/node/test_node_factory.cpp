# include "test_node_factory.hpp"

using namespace std::string_literals;

void test_newNodeNoArgs() {
    auto node = newNode();
}

void test_newNodeNoArgsHasNoneData() {
    auto node = newNode();
    std::cout << node << std::endl;
    if (node->noData() == false) {
        throw py::value_error("expected no data in default node construction");
    }
}


void test_newNodeOnlyName() {
    auto node = newNode("TheName"s);

    if (node->name() != "TheName") {
        throw py::value_error("failed Node construction");
    }
}

void test_newNodeOnlyNameChars() {
    auto node = newNode("TheName");

    if (node->name() != "TheName") {
        throw py::value_error("failed Node construction");
    }
}

void test_newNodeNameAndType() {
    auto node = newNode("TheName"s, "TheType"s);

    if (node->name() != "TheName") {
        throw py::value_error("failed Node construction, wrong name");
    }

    if (node->type() != "TheType") {
        throw py::value_error("failed Node construction, wrong type");
    }
}

void test_newNodeNameTypeAndData() {
    auto node = newNode("TheName"s, "TheType"s, 1);

    if (node->name() != "TheName") {
        throw py::value_error("failed Node construction, wrong name");
    }

    if (node->type() != "TheType") {
        throw py::value_error("failed Node construction, wrong type");
    }

    std::shared_ptr<Array> arr = std::dynamic_pointer_cast<Array>(node->dataPtr());
    if (!arr) throw std::runtime_error("Node data is not an Array");
    if (*arr != 1) {
        throw py::value_error("failed Node construction, wrong data");
    }
}

void test_newNodeDataString() {
    auto node = newNode("TheName", "TheType", "MyData"s);
    
    // recommended approach
    if (node->data().extractString() != "MyData") {
        throw py::value_error("failed Node construction, wrong data (natural approach)");
    }

    // by array extraction array approach (more complicated)
    std::shared_ptr<Array> arr = std::dynamic_pointer_cast<Array>(node->dataPtr());
    if (!arr) throw std::runtime_error("Node data is not an Array");

    if (*arr != "MyData") {
        throw py::value_error("failed Node construction, wrong data (by array extraction)");
    }
}


void test_newNodeDataStringChars() {
    auto node = newNode("TheName", "TheType", "MyData");
    
    // recommended approach
    if (node->data().extractString() != "MyData") {
        throw py::value_error("failed Node construction, wrong data (natural approach)");
    }

    // by array extraction array approach (more complicated)
    std::shared_ptr<Array> arr = std::dynamic_pointer_cast<Array>(node->dataPtr());
    if (!arr) throw std::runtime_error("Node data is not an Array");

    if (*arr != "MyData") {
        throw py::value_error("failed Node construction, wrong data (by array extraction)");
    }
}

void test_newNodeParent() {
    auto a = newNode("a");
    auto b = newNode("b","t",false,a);
    auto b_picked = a->children()[0];

    if (b_picked->name() != "b") throw py::value_error("failed attaching parent");
}