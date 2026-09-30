#ifndef TEST_PREDICATE_EXPLORER_PYBIND_HPP
#define TEST_PREDICATE_EXPLORER_PYBIND_HPP

#include <pybind11/pybind11.h>

#include "test_predicate_explorer.hpp"

void bindTestsOfPredicateExplorer(py::module_& m) {
    py::module_ submodule = m.def_submodule("predicate_explorer");
    submodule.def("traversal", &test_predicate_explorer_traversal);
    submodule.def("data_and_levels", &test_predicate_explorer_data_and_levels);
    submodule.def("boolean_and_quoting", &test_predicate_explorer_boolean_and_quoting);
    submodule.def("parser_errors", &test_predicate_explorer_parser_errors);
}

#endif // TEST_PREDICATE_EXPLORER_PYBIND_HPP
