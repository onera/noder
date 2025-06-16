#ifndef TEST_NAVIGATION_H
#define TEST_NAVIGATION_H

#include <node/node.hpp>

# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

namespace py = pybind11;

void test_childByName();

void test_byName();

void test_byNamePattern();

void test_childByType();

void test_byType();

void test_byTypePattern();

#endif // TEST_NAVIGATION_H