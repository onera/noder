#ifndef TEST_PREDICATE_EXPLORER_H
#define TEST_PREDICATE_EXPLORER_H

#include <node/node.hpp>
#include <node/node_factory.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void test_predicate_explorer_traversal();

void test_predicate_explorer_data_and_levels();

void test_predicate_explorer_boolean_and_quoting();

void test_predicate_explorer_parser_errors();

#endif // TEST_PREDICATE_EXPLORER_H
