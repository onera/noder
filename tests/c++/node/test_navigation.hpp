#ifndef TEST_NAVIGATION_H
#define TEST_NAVIGATION_H

#include <node/node.hpp>

# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

# include <node/node_factory.hpp>

namespace py = pybind11;

void test_childByName();

void test_byName();

void test_byNamePattern();

void test_childByType();

void test_byType();

void test_byTypePattern();

void test_childByData();

void test_childByDataUsingChar();

void test_childByDataScalarDirect();

void test_byData();

#endif // TEST_NAVIGATION_H
