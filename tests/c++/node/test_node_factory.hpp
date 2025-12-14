#ifndef TEST_NODE_FACTORY_H
#define TEST_NODE_FACTORY_H

# include <node/node.hpp>
# include <node/node_factory.hpp>

void test_newNodeNoArgs();
void test_newNodeOnlyName();
void test_newNodeOnlyNameChars();
void test_newNodeNameAndType();
void test_newNodeNameTypeAndData();
void test_newNodeDataString();
void test_newNodeDataStringChars();
void test_newNodeParent();

#endif