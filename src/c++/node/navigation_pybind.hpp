# include "node/navigation.hpp"
# include "node/node.hpp"
# include <pybind11/numpy.h>
# include "utils/template_binder.hpp"

namespace {

std::shared_ptr<Node> tryChildByDataMethod(
    py::object& selfObject,
    const char* methodName,
    const py::object& data) {
    try {
        py::object result = selfObject.attr(methodName)(data);
        if (!result.is_none()) {
            return result.cast<std::shared_ptr<Node>>();
        }
    } catch (const py::error_already_set& e) {
        if (!e.matches(PyExc_TypeError) &&
            !e.matches(PyExc_OverflowError) &&
            !e.matches(PyExc_ValueError)) {
            throw;
        }
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<Node> dispatchChildByData(Navigation& self, const py::object& data) {
    py::object selfObject = py::cast(&self, py::return_value_policy::reference);

    if (py::isinstance<py::str>(data)) {
        return self.childByData(data.cast<std::string>());
    }

    // bool must be checked before int (Python bool is an int subclass)
    if (py::isinstance<py::bool_>(data)) {
        return tryChildByDataMethod(selfObject, "child_by_data_bool", data);
    }

    if (py::isinstance<py::int_>(data)) {
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_int64", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_uint64", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_int32", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_uint32", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_int16", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_uint16", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_int8", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_uint8", data)) return n;
        return nullptr;
    }

    if (py::isinstance<py::float_>(data)) {
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_double", data)) return n;
        if (auto n = tryChildByDataMethod(selfObject, "child_by_data_float", data)) return n;
        return nullptr;
    }

    // NumPy scalars (and size-1 arrays) can usually be unwrapped through item().
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dispatchChildByData(self, scalar);
            }
        } catch (const py::error_already_set&) {
            // Fall through to unified type error below.
        }
    }

    throw py::type_error(
        "child_by_data: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}

} // namespace

void bindNavigation(py::module_ &m) {
    auto navigation = py::class_<Navigation>(m, "Navigation");

    navigation
        .def("child_by_name", &Navigation::childByName, "Get child node by name")
        .def("by_name",
             py::overload_cast<const std::string&, const int&>(&Navigation::byName),
             "get node by exact name recursively",
             py::arg("name"), py::arg("depth")=100)
        .def("by_name_pattern",
             py::overload_cast<const std::string&, const int&>(&Navigation::byNamePattern),
             "get node by regex-pattern name recursively",
             py::arg("name_pattern"), py::arg("depth")=100)
        .def("child_by_type", &Navigation::childByType, "Get child node by type")
        .def("by_type",
             py::overload_cast<const std::string&, const int&>(&Navigation::byType),
             "get node by exact type recursively",
             py::arg("type"), py::arg("depth")=100)
        .def("by_type_pattern",
             py::overload_cast<const std::string&, const int&>(&Navigation::byTypePattern),
             "get node by regex-pattern type recursively",
             py::arg("type_pattern"), py::arg("depth")=100)
        .def("child_by_data",
             [](Navigation& self, const py::object& data) {
                 return dispatchChildByData(self, data);
             },
             "Get child node by data (string or scalar)",
             py::arg("data"));

    utils::bindClassMethodForScalarTypes(
        navigation,
        "child_by_data",
        []<typename T>(utils::TypeTag<T>) {
            return [](Navigation& self, T data) {
                return self.template childByData<T>(data);
            };
        },
        py::arg("data"),
        "Get child node by scalar data (typed overload)");

}
