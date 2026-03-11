#ifndef TEST_ZONE_PYBIND_HPP
#define TEST_ZONE_PYBIND_HPP

#include <pybind11/pybind11.h>

#include "test_zone.hpp"

void bindTestsOfZone(py::module_& m) {
    py::module_ sm = m.def_submodule("zone");
    sm.def("test_zone_init", &test_zone_init);
    sm.def("test_zone_structure_flags", &test_zone_structure_flags);
    sm.def("test_zone_metrics", &test_zone_metrics);
    sm.def("test_zone_new_fields", &test_zone_new_fields);
    sm.def("test_zone_fields_accessors", &test_zone_fields_accessors);
    sm.def("test_zone_all_fields_and_xyz", &test_zone_all_fields_and_xyz);
    sm.def("test_zone_infer_location_and_coherency", &test_zone_infer_location_and_coherency);
    sm.def("test_zone_update_shape", &test_zone_update_shape);
    sm.def("test_zone_is_empty", &test_zone_is_empty);
    sm.def("test_zone_boundaries", &test_zone_boundaries);
}

#endif
