#ifndef TEST_YAML_IO_H
#define TEST_YAML_IO_H

#include <array/factory/matrices.hpp>
#include <array/factory/vectors.hpp>
#include <io/io.hpp>
#include <node/node_factory.hpp>

#include <memory>
#include <string>

using namespace std::string_literals;
using namespace io;
using namespace arrayfactory;

namespace test_io {

inline std::shared_ptr<Node> make_yaml_test_tree() {
    auto root = newNode("root", "UserDefinedData_t");

    auto integers = newNode("integers", "DataArray_t");
    integers->setData(uniformFromStep<int32_t>(0, 6));
    integers->attachTo(root);

    auto matrix = newNode("matrix", "DataArray_t");
    matrix->setData(arrayfactory::ones<double>({2, 3}, 'F'));
    matrix->attachTo(root);

    auto message = newNode("message", "Descriptor_t");
    message->setData("hola");
    message->attachTo(root);

    return root;
}

inline void test_write_yaml_nodes(std::string filename = "test.yaml") {
    write_node(filename, make_yaml_test_tree());
}

inline std::shared_ptr<Node> test_read_yaml(std::string filename = "test_read.yaml") {
    test_write_yaml_nodes(filename);
    return read(filename);
}

inline void test_write_yaml_link_nodes(std::string filename = "test_links.yaml") {
    auto root = newNode("root", "UserDefinedData_t");
    auto target = newNode("target", "DataArray_t");
    target->setData(uniformFromStep<int32_t>(0, 4));
    target->attachTo(root);

    auto linkNode = newNode("target_link", "DataArray_t");
    linkNode->setLinkTarget(".", "/root/target");
    linkNode->attachTo(root);

    write_node(filename, root);
}

inline std::shared_ptr<Node> test_read_yaml_links(std::string filename = "test_read_links.yaml") {
    test_write_yaml_link_nodes(filename);
    return read(filename);
}

inline std::shared_ptr<Node> test_read_yaml_cgns_tree(std::string filename = "test_tree.yaml") {
    auto tree = newNode("CGNSTree", "CGNSTree_t");
    auto version = newNode("CGNSLibraryVersion", "CGNSLibraryVersion_t");
    version->setData(4.0f);
    version->attachTo(tree);

    auto base = newNode("Base", "CGNSBase_t");
    base->setData(uniformFromStep<int32_t>(1, 3));
    base->attachTo(tree);

    write_node(filename, tree);
    return read(filename);
}

} // namespace test_io

#endif // TEST_YAML_IO_H
