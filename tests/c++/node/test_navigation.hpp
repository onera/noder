#ifndef TEST_NAVIGATION_H
#define TEST_NAVIGATION_H

# include <node/node.hpp>
# include <node/node_factory.hpp>

# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>


namespace py = pybind11;

void test_childByName();

void test_byName();

void test_allByName();

void test_byNameRegex();

void test_allByNameRegex();

void test_byNameGlob();

void test_allByNameGlob();

void test_childByType();

void test_byType();

void test_allByType();

void test_byTypeRegex();

void test_allByTypeRegex();

void test_byTypeGlob();

void test_allByTypeGlob();

void test_childByData();

void test_childByDataUsingChar();

void test_childByDataScalarDirect();

void test_byData();

void test_allByData();

void test_byDataGlob();

void test_allByDataGlob();

void test_byDataScalarDirect();

void test_allByDataScalarDirect();

#endif // TEST_NAVIGATION_H
