#ifndef BASE_PYBIND_HPP
#define BASE_PYBIND_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "cgns/base.hpp"
#include "cgns/cgns_pybind_utils.hpp"

namespace py = pybind11;

void bindBase(py::module_& m) {
    py::class_<Base, Node, std::shared_ptr<Base>>(
        m,
        "Base",
        R"doc(
CGNS Base helper built on top of :py:class:`noder.core.Node`.

This class provides treelab-inspired zone aggregation helpers.
)doc")
        .def(py::init<const std::string&>(), py::arg("name") = "Base")
        .def("dim", &Base::dim)
        .def("physical_dimension", &Base::physicalDimension)
        .def("set_cell_dimension", &Base::setCellDimension, py::arg("cell_dimension"))
        .def("set_physical_dimension", &Base::setPhysicalDimension, py::arg("physical_dimension"))
        .def("zones", &Base::zones)
        .def("is_structured", &Base::isStructured)
        .def("is_unstructured", &Base::isUnstructured)
        .def("is_hybrid", &Base::isHybrid)
        .def("number_of_points", &Base::numberOfPoints)
        .def("number_of_cells", &Base::numberOfCells)
        .def("number_of_zones", &Base::numberOfZones)
        .def("get_elements_types", &Base::getElementsTypes)
        .def(
            "new_fields",
            [](Base& base,
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

                auto output = base.newFields(
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
            [](Base& base,
               const py::object& fieldNames,
               const std::string& container,
               const std::string& behaviorIfNotFound,
               const py::object& dtype,
               const std::string& returnType,
               bool ravel) {
                const auto names = cgns_pybind_utils::fieldNamesFromObject(fieldNames, "fields");
                auto output = base.fields(
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
            [](const Base& base,
               bool includeCoordinates,
               const std::string& returnType,
               bool ravel,
               bool appendContainerToFieldName) {
                auto output = base.allFields(includeCoordinates, appendContainerToFieldName, ravel);
                return cgns_pybind_utils::aggregatedNamedDataListToObject(output, returnType);
            },
            py::arg("include_coordinates") = true,
            py::arg("return_type") = "dict",
            py::arg("ravel") = false,
            py::arg("append_container_to_field_name") = false)
        .def("assert_fields_size_coherency", &Base::assertFieldsSizeCoherency)
        .def("remove_empty_zones", &Base::removeEmptyZones)
        .def("update_dimensions_from_zones", &Base::updateDimensionsFromZones);

    m.def(
        "new_base",
        &newBase,
        py::arg("name"),
        py::arg("zones"),
        R"doc(
Construct a CGNSBase_t node from a list of zones.

The base dimensions are inferred from the zones and must be coherent across
all provided zones.
)doc");
}

#endif
