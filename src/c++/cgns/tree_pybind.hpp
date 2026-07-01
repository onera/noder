#ifndef TREE_PYBIND_HPP
#define TREE_PYBIND_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "cgns/cgns_pybind_utils.hpp"
#include "cgns/tree.hpp"

namespace py = pybind11;

void bindTree(py::module_& m) {
    py::class_<Tree, Node, std::shared_ptr<Tree>>(
        m,
        "Tree",
        R"doc(
CGNS Tree helper built on top of :py:class:`noder.core.Node`.

This class provides treelab-inspired base and zone aggregation helpers.
)doc")
        .def(py::init<>())
        .def("bases", &Tree::bases)
        .def("base_names", &Tree::baseNames)
        .def("base", &Tree::base, py::arg("name"))
        .def("set_unique_base_names", &Tree::setUniqueBaseNames)
        .def("set_unique_zone_names", &Tree::setUniqueZoneNames)
        .def("add", &Tree::add, py::arg("node"))
        .def("zones", &Tree::zones)
        .def("zone_names", &Tree::zoneNames)
        .def("is_structured", &Tree::isStructured)
        .def("is_unstructured", &Tree::isUnstructured)
        .def("is_hybrid", &Tree::isHybrid)
        .def("number_of_points", &Tree::numberOfPoints)
        .def("number_of_cells", &Tree::numberOfCells)
        .def("number_of_zones", &Tree::numberOfZones)
        .def("number_of_bases", &Tree::numberOfBases)
        .def("get_elements_types", &Tree::getElementsTypes)
        .def(
            "new_fields",
            [](Tree& tree,
               const py::object& inputFields,
               const std::string& container,
               const std::string& gridLocation,
               const py::object& dtype,
               const std::string& returnType,
               bool ravel) {
                const auto specs = cgns_pybind_utils::fieldSpecsFromObject(inputFields, "new_fields");
                std::vector<std::string> names;
                names.reserve(specs.size());
                for (const auto& [name, value] : specs) {
                    (void)value;
                    names.push_back(name);
                }

                auto output = tree.newFields(
                    specs,
                    container,
                    gridLocation,
                    cgns_pybind_utils::dtypeNameFromObject(dtype),
                    ravel);
                return cgns_pybind_utils::aggregatedDataListToObject(output, names, returnType);
            },
            py::arg("input_fields"),
            py::arg("container") = "FlowSolution",
            py::arg("grid_location") = "auto",
            py::arg("dtype") = py::str("float64"),
            py::arg("return_type") = "dict",
            py::arg("ravel") = false)
        .def(
            "fields",
            [](Tree& tree,
               const py::object& fieldNames,
               const std::string& container,
               const std::string& behaviorIfNotFound,
               const py::object& dtype,
               const std::string& returnType,
               bool ravel) {
                const auto names = cgns_pybind_utils::fieldNamesFromObject(fieldNames, "fields");
                auto output = tree.fields(
                    names,
                    container,
                    behaviorIfNotFound,
                    cgns_pybind_utils::dtypeNameFromObject(dtype),
                    ravel);
                return cgns_pybind_utils::aggregatedDataListToObject(output, names, returnType);
            },
            py::arg("field_names"),
            py::arg("container") = "FlowSolution",
            py::arg("behavior_if_not_found") = "create",
            py::arg("dtype") = py::str("float64"),
            py::arg("return_type") = "dict",
            py::arg("ravel") = false)
        .def(
            "all_fields",
            [](const Tree& tree,
               bool includeCoordinates,
               const std::string& returnType,
               bool ravel,
               bool appendContainerToFieldName) {
                auto output = tree.allFields(includeCoordinates, appendContainerToFieldName, ravel);
                return cgns_pybind_utils::aggregatedNamedDataListToObject(output, returnType);
            },
            py::arg("include_coordinates") = true,
            py::arg("return_type") = "dict",
            py::arg("ravel") = false,
            py::arg("append_container_to_field_name") = false)
        .def("assert_fields_size_coherency", &Tree::assertFieldsSizeCoherency)
        .def("remove_empty_zones", &Tree::removeEmptyZones);
}

#endif
