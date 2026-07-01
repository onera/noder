#ifndef CGNS_PYBIND_UTILS_HPP
#define CGNS_PYBIND_UTILS_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "array/array.hpp"
#include "array/array_numpy_bridge.hpp"
#include "cgns/zone.hpp"
#include "data/data.hpp"

namespace py = pybind11;

namespace cgns_pybind_utils {

inline py::object objectFromData(const std::shared_ptr<Data>& data) {
    if (!data || data->isNone()) {
        return py::none();
    }

    auto arrayData = std::dynamic_pointer_cast<Array>(data);
    if (arrayData) {
        return arraybridge::toPyObject(*arrayData);
    }

    return py::cast(data);
}

inline std::vector<std::string> fieldNamesFromObject(const py::object& fieldNames, const char* context) {
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

inline std::vector<std::pair<std::string, double>> fieldSpecsFromObject(
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
            specs.emplace_back(item.first.cast<std::string>(), py::cast<double>(item.second));
        }
        return specs;
    }

    throw py::type_error(std::string(context) + ": expected str, list[str], or dict[str, scalar]");
}

inline std::string dtypeNameFromObject(const py::object& dtypeObject) {
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

inline py::object dataListToObject(
    const Zone::DataList& dataList,
    const std::vector<std::string>& fieldNames,
    const std::string& returnType,
    bool collapseSingle) {

    if (returnType == "dict") {
        py::dict output;
        const size_t size = std::min(fieldNames.size(), dataList.size());
        for (size_t i = 0; i < size; ++i) {
            output[py::str(fieldNames[i])] = objectFromData(dataList[i]);
        }
        return output;
    }

    if (returnType != "list") {
        throw py::value_error("return_type must be 'list' or 'dict'");
    }

    if (collapseSingle && dataList.size() == 1) {
        return objectFromData(dataList.front());
    }

    py::list output;
    for (const auto& data : dataList) {
        output.append(objectFromData(data));
    }
    return output;
}

inline py::object namedDataListToObject(const Zone::NamedDataList& data, const std::string& returnType) {
    if (returnType == "list") {
        py::list output;
        for (const auto& [name, value] : data) {
            (void)name;
            output.append(objectFromData(value));
        }
        return output;
    }

    if (returnType == "dict") {
        py::dict output;
        for (const auto& [name, value] : data) {
            output[py::str(name)] = objectFromData(value);
        }
        return output;
    }

    throw py::value_error("return_type must be 'list' or 'dict'");
}

template <typename AggregatedDataList>
inline py::object aggregatedDataListToObject(
    const AggregatedDataList& data,
    const std::vector<std::string>& fieldNames,
    const std::string& returnType) {

    if (returnType == "list") {
        py::list output;
        for (const auto& [zoneName, values] : data) {
            (void)zoneName;
            output.append(dataListToObject(values, fieldNames, returnType, true));
        }
        return output;
    }

    if (returnType == "dict") {
        py::dict output;
        for (const auto& [zoneName, values] : data) {
            output[py::str(zoneName)] = dataListToObject(values, fieldNames, returnType, false);
        }
        return output;
    }

    throw py::value_error("return_type must be 'list' or 'dict'");
}

template <typename AggregatedNamedDataList>
inline py::object aggregatedNamedDataListToObject(
    const AggregatedNamedDataList& data,
    const std::string& returnType) {

    if (returnType == "list") {
        py::list output;
        for (const auto& [zoneName, values] : data) {
            (void)zoneName;
            output.append(namedDataListToObject(values, returnType));
        }
        return output;
    }

    if (returnType == "dict") {
        py::dict output;
        for (const auto& [zoneName, values] : data) {
            output[py::str(zoneName)] = namedDataListToObject(values, returnType);
        }
        return output;
    }

    throw py::value_error("return_type must be 'list' or 'dict'");
}

} // namespace cgns_pybind_utils

#endif
