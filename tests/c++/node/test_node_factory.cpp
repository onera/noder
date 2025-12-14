# include "test_node_factory.hpp"

using namespace std::string_literals;

void test_newNodeNoArgs() {
    // call without arguments requires type instantiation since
    // it cannot guess data type
    auto node = newNode<int>();
}

void test_newNodeOnlyName() {
    auto node = newNode<int>("TheName"s);
}

void test_newNodeNameAndType() {
    auto node = newNode<int>("TheName"s, "TheType"s);
}

void test_newNodeNameTypeAndData() {
    // should be able to automatically find type
    auto node = newNode("TheName"s, "TheType"s, 1);
}