#ifndef TEST_BASE_TREE_PYBIND_HPP
#define TEST_BASE_TREE_PYBIND_HPP

#include <pybind11/pybind11.h>

#include "test_base_tree.hpp"

void bindTestsOfBaseTree(py::module_& m) {
    py::module_ sm = m.def_submodule("base_tree");
    sm.def("test_base_new_base_dimensions", &test_base_new_base_dimensions);
    sm.def("test_base_aggregates_zones", &test_base_aggregates_zones);
    sm.def("test_tree_aggregates_bases_and_zones", &test_tree_aggregates_bases_and_zones);
    sm.def("test_new_base_rejects_incoherent_dimensions", &test_new_base_rejects_incoherent_dimensions);
    sm.def("test_newZoneFromArrays", &test_newZoneFromArrays);
}

#endif
