# ifndef TEST_NODE_GROUP_HPP
# define TEST_NODE_GROUP_HPP

# include <node/node.hpp>
# include <node/node_factory.hpp>

# include <pybind11/pybind11.h>

namespace py = pybind11;

void test_operator_plus_attachToFirstParent();

void test_operator_div_addChild_chain();

void test_operator_div_with_plus_group();

void test_operator_nested_expression_tree();

# endif
