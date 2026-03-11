#ifndef ZONE_PYBIND_HPP
#define ZONE_PYBIND_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "array/array.hpp"
#include "cgns/zone.hpp"

namespace py = pybind11;

namespace {

py::object pyObjectFromData(const std::shared_ptr<Data>& data) {
    if (!data || data->isNone()) {
        return py::none();
    }

    auto arrayData = std::dynamic_pointer_cast<Array>(data);
    if (arrayData) {
        return arrayData->getPyArray();
    }

    return py::cast(data);
}

std::vector<std::string> fieldNamesFromPyObject(const py::object& fieldNames, const char* context) {
    if (py::isinstance<py::str>(fieldNames)) {
        return {fieldNames.cast<std::string>()};
    }

    if (py::isinstance<py::list>(fieldNames) || py::isinstance<py::tuple>(fieldNames)) {
        std::vector<std::string> names;
        py::sequence sequence = fieldNames.cast<py::sequence>();
        names.reserve(py::len(sequence));
        for (const auto& item : sequence) {
            if (!py::isinstance<py::str>(item)) {
                throw py::type_error(std::string(context) + ": field names must be strings");
            }
            names.push_back(py::reinterpret_borrow<py::str>(item).cast<std::string>());
        }
        return names;
    }

    throw py::type_error(std::string(context) + ": expected str or list/tuple of str");
}

std::vector<std::pair<std::string, double>> fieldSpecsFromPyObject(
    const py::object& inputFields,
    const char* context) {

    if (py::isinstance<py::str>(inputFields)) {
        return {{inputFields.cast<std::string>(), 0.0}};
    }

    if (py::isinstance<py::list>(inputFields) || py::isinstance<py::tuple>(inputFields)) {
        std::vector<std::pair<std::string, double>> specs;
        py::sequence sequence = inputFields.cast<py::sequence>();
        specs.reserve(py::len(sequence));
        for (const auto& item : sequence) {
            if (!py::isinstance<py::str>(item)) {
                throw py::type_error(std::string(context) + ": list items must be field-name strings");
            }
            specs.emplace_back(py::reinterpret_borrow<py::str>(item).cast<std::string>(), 0.0);
        }
        return specs;
    }

    if (py::isinstance<py::dict>(inputFields)) {
        std::vector<std::pair<std::string, double>> specs;
        py::dict dictionary = inputFields.cast<py::dict>();
        specs.reserve(dictionary.size());
        for (const auto& item : dictionary) {
            if (!py::isinstance<py::str>(item.first)) {
                throw py::type_error(std::string(context) + ": dict keys must be strings");
            }
            const std::string name = item.first.cast<std::string>();
            const double value = py::cast<double>(item.second);
            specs.emplace_back(name, value);
        }
        return specs;
    }

    throw py::type_error(std::string(context) + ": expected str, list[str], or dict[str, scalar]");
}

std::string dtypeNameFromPyObject(const py::object& dtypeObject) {
    if (dtypeObject.is_none()) {
        return "float64";
    }
    if (py::isinstance<py::str>(dtypeObject)) {
        return dtypeObject.cast<std::string>();
    }

    py::module_ np = py::module_::import("numpy");
    py::object dtype = np.attr("dtype")(dtypeObject);
    return dtype.attr("name").cast<std::string>();
}

py::object pyObjectFromDataList(
    const Zone::DataList& dataList,
    const std::vector<std::string>& fieldNames,
    const std::string& returnType,
    bool collapseSingle) {

    if (returnType == "dict") {
        py::dict output;
        const size_t size = std::min(fieldNames.size(), dataList.size());
        for (size_t i = 0; i < size; ++i) {
            output[py::str(fieldNames[i])] = pyObjectFromData(dataList[i]);
        }
        return output;
    }

    if (returnType != "list") {
        throw py::value_error("return_type must be 'list' or 'dict'");
    }

    if (collapseSingle && dataList.size() == 1) {
        return pyObjectFromData(dataList.front());
    }

    py::list output;
    for (const auto& data : dataList) {
        output.append(pyObjectFromData(data));
    }
    return output;
}

py::object pyObjectFromNamedDataList(const Zone::NamedDataList& data, const std::string& returnType) {
    if (returnType == "list") {
        py::list output;
        for (const auto& [name, value] : data) {
            (void)name;
            output.append(pyObjectFromData(value));
        }
        return output;
    }

    if (returnType == "dict") {
        py::dict output;
        for (const auto& [name, value] : data) {
            output[py::str(name)] = pyObjectFromData(value);
        }
        return output;
    }

    throw py::value_error("return_type must be 'list' or 'dict'");
}

py::object pyObjectFromCoordinates(const Zone::DataList& coordinates, const std::string& returnType) {
    if (coordinates.size() < 3) {
        throw py::value_error("coordinates accessor returned fewer than 3 arrays");
    }

    if (returnType == "list") {
        py::list output;
        output.append(pyObjectFromData(coordinates[0]));
        output.append(pyObjectFromData(coordinates[1]));
        output.append(pyObjectFromData(coordinates[2]));
        return output;
    }

    if (returnType == "dict") {
        py::dict output;
        output[py::str("CoordinateX")] = pyObjectFromData(coordinates[0]);
        output[py::str("CoordinateY")] = pyObjectFromData(coordinates[1]);
        output[py::str("CoordinateZ")] = pyObjectFromData(coordinates[2]);
        return output;
    }

    throw py::value_error("return_type must be 'list' or 'dict'");
}

} // namespace

void bindZone(py::module_& m) {
    py::class_<Zone, Node, std::shared_ptr<Zone>>(
        m,
        "Zone",
        R"doc(
CGNS Zone helper built on top of :py:class:`noder.core.Node`.

This class provides treelab-inspired field and geometry helpers.
Not implemented on purpose: ``save`` and ``useEquation``.
)doc")
        .def(
            py::init([](const std::string& name) {
                auto zone = std::make_shared<Zone>(name);
                // Ensure ZoneType child exists for heap-managed instances.
                zone->isStructured();
                return zone;
            }),
            py::arg("name") = "Zone")
        .def("is_structured", &Zone::isStructured)
        .def("is_unstructured", &Zone::isUnstructured)
        .def("get_elements_types", &Zone::getElementsTypes)
        .def("dim", &Zone::dim)
        .def("shape", &Zone::shape)
        .def("shape_of_coordinates", &Zone::shapeOfCoordinates)
        .def("number_of_points", &Zone::numberOfPoints)
        .def("number_of_cells", &Zone::numberOfCells)
        .def(
            "new_fields",
            [](Zone& zone,
               const py::object& inputFields,
               const std::string& container,
               const std::string& gridLocation,
               const py::object& dtype,
               const std::string& returnType,
               bool ravel) {
                const auto specs = fieldSpecsFromPyObject(inputFields, "new_fields");
                auto output = zone.newFields(
                    specs,
                    container,
                    gridLocation,
                    dtypeNameFromPyObject(dtype),
                    ravel);

                std::vector<std::string> names;
                names.reserve(specs.size());
                for (const auto& [name, value] : specs) {
                    (void)value;
                    names.push_back(name);
                }
                return pyObjectFromDataList(output, names, returnType, true);
            },
            py::arg("input_fields"),
            py::arg("container") = "FlowSolution",
            py::arg("grid_location") = "auto",
            py::arg("dtype") = py::str("float64"),
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def(
            "remove_fields",
            [](Zone& zone, const py::object& fieldNames, const std::string& container) {
                zone.removeFields(fieldNamesFromPyObject(fieldNames, "remove_fields"), container);
            },
            py::arg("field_names"),
            py::arg("container") = "FlowSolution")
        .def(
            "fields",
            [](Zone& zone,
               const py::object& fieldNames,
               const std::string& container,
               const std::string& behaviorIfNotFound,
               const py::object& dtype,
               const std::string& returnType,
               bool ravel) -> py::object {
                const auto names = fieldNamesFromPyObject(fieldNames, "fields");
                auto output = zone.fields(
                    names,
                    container,
                    behaviorIfNotFound,
                    dtypeNameFromPyObject(dtype),
                    ravel);

                if (behaviorIfNotFound == "pass" && output.empty()) {
                    return py::none();
                }
                return pyObjectFromDataList(output, names, returnType, true);
            },
            py::arg("field_names"),
            py::arg("container") = "FlowSolution",
            py::arg("behavior_if_not_found") = "create",
            py::arg("dtype") = py::str("float64"),
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def(
            "field",
            [](Zone& zone,
               const std::string& fieldName,
               const std::string& container,
               const std::string& behaviorIfNotFound,
               const py::object& dtype,
               bool ravel) {
                auto output = zone.field(
                    fieldName,
                    container,
                    behaviorIfNotFound,
                    dtypeNameFromPyObject(dtype),
                    ravel);
                return pyObjectFromData(output);
            },
            py::arg("field_name"),
            py::arg("container") = "FlowSolution",
            py::arg("behavior_if_not_found") = "create",
            py::arg("dtype") = py::str("float64"),
            py::arg("ravel") = false)
        .def(
            "xyz",
            [](const Zone& zone, const std::string& returnType, bool ravel) {
                return pyObjectFromCoordinates(zone.xyz(ravel), returnType);
            },
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def(
            "xy",
            [](const Zone& zone, const std::string& returnType, bool ravel) -> py::object {
                const auto values = zone.xy(ravel);
                if (returnType == "list") {
                    py::list output;
                    output.append(pyObjectFromData(values[0]));
                    output.append(pyObjectFromData(values[1]));
                    return output;
                }
                if (returnType == "dict") {
                    py::dict output;
                    output[py::str("CoordinateX")] = pyObjectFromData(values[0]);
                    output[py::str("CoordinateY")] = pyObjectFromData(values[1]);
                    return output;
                }
                throw py::value_error("return_type must be 'list' or 'dict'");
            },
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def(
            "xz",
            [](const Zone& zone, const std::string& returnType, bool ravel) -> py::object {
                const auto values = zone.xz(ravel);
                if (returnType == "list") {
                    py::list output;
                    output.append(pyObjectFromData(values[0]));
                    output.append(pyObjectFromData(values[1]));
                    return output;
                }
                if (returnType == "dict") {
                    py::dict output;
                    output[py::str("CoordinateX")] = pyObjectFromData(values[0]);
                    output[py::str("CoordinateZ")] = pyObjectFromData(values[1]);
                    return output;
                }
                throw py::value_error("return_type must be 'list' or 'dict'");
            },
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def(
            "yz",
            [](const Zone& zone, const std::string& returnType, bool ravel) -> py::object {
                const auto values = zone.yz(ravel);
                if (returnType == "list") {
                    py::list output;
                    output.append(pyObjectFromData(values[0]));
                    output.append(pyObjectFromData(values[1]));
                    return output;
                }
                if (returnType == "dict") {
                    py::dict output;
                    output[py::str("CoordinateY")] = pyObjectFromData(values[0]);
                    output[py::str("CoordinateZ")] = pyObjectFromData(values[1]);
                    return output;
                }
                throw py::value_error("return_type must be 'list' or 'dict'");
            },
            py::arg("return_type") = "list",
            py::arg("ravel") = false)
        .def("x", [](const Zone& zone, bool ravel) { return pyObjectFromData(zone.x(ravel)); }, py::arg("ravel") = false)
        .def("y", [](const Zone& zone, bool ravel) { return pyObjectFromData(zone.y(ravel)); }, py::arg("ravel") = false)
        .def("z", [](const Zone& zone, bool ravel) { return pyObjectFromData(zone.z(ravel)); }, py::arg("ravel") = false)
        .def(
            "all_fields",
            [](const Zone& zone,
               bool includeCoordinates,
               const std::string& returnType,
               bool ravel,
               bool appendContainerToFieldName) {
                auto data = zone.allFields(includeCoordinates, appendContainerToFieldName, ravel);
                return pyObjectFromNamedDataList(data, returnType);
            },
            py::arg("include_coordinates") = true,
            py::arg("return_type") = "dict",
            py::arg("ravel") = false,
            py::arg("append_container_to_field_name") = false)
        .def("assert_fields_size_coherency", &Zone::assertFieldsSizeCoherency)
        .def("infer_location", &Zone::inferLocation, py::arg("container"))
        .def("has_fields", &Zone::hasFields)
        .def("get_array_shapes", &Zone::getArrayShapes)
        .def("update_shape", &Zone::updateShape)
        .def("is_empty", &Zone::isEmpty)
        .def("boundaries", &Zone::boundaries)
        .def("boundary", &Zone::boundary, py::arg("index") = "i", py::arg("bound") = "min")
        .def("imin", &Zone::imin)
        .def("imax", &Zone::imax)
        .def("jmin", &Zone::jmin)
        .def("jmax", &Zone::jmax)
        .def("kmin", &Zone::kmin)
        .def("kmax", &Zone::kmax);
}

#endif
