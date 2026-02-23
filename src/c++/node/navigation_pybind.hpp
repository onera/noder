# include "node/navigation.hpp"
# include "node/node.hpp"
# include <pybind11/numpy.h>
# include <pybind11/stl.h>
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

std::shared_ptr<Node> tryByDataMethod(
    py::object& selfObject,
    const char* methodName,
    const py::object& data,
    const size_t& depth) {
    try {
        py::object result = selfObject.attr(methodName)(data, depth);
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

bool tryAllByDataMethod(
    py::object& selfObject,
    const char* methodName,
    const py::object& data,
    const size_t& depth,
    std::vector<std::shared_ptr<Node>>& output) {
    try {
        py::object result = selfObject.attr(methodName)(data, depth);
        output = result.cast<std::vector<std::shared_ptr<Node>>>();
        return true;
    } catch (const py::error_already_set& e) {
        if (!e.matches(PyExc_TypeError) &&
            !e.matches(PyExc_OverflowError) &&
            !e.matches(PyExc_ValueError)) {
            throw;
        }
        return false;
    }
}

bool tryAllByAndMethod(
    py::object& selfObject,
    const char* methodName,
    const std::string& name,
    const std::string& type,
    const py::object& data,
    const size_t& depth,
    std::vector<std::shared_ptr<Node>>& output) {
    try {
        py::object result = selfObject.attr(methodName)(name, type, data, depth);
        output = result.cast<std::vector<std::shared_ptr<Node>>>();
        return true;
    } catch (const py::error_already_set& e) {
        if (!e.matches(PyExc_TypeError) &&
            !e.matches(PyExc_OverflowError) &&
            !e.matches(PyExc_ValueError)) {
            throw;
        }
        return false;
    }
}

std::shared_ptr<Node> tryByAndMethod(
    py::object& selfObject,
    const char* methodName,
    const std::string& name,
    const std::string& type,
    const py::object& data,
    const size_t& depth) {
    try {
        py::object result = selfObject.attr(methodName)(name, type, data, depth);
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

std::shared_ptr<Node> dispatchByData(
    Navigation& self,
    const py::object& data,
    const size_t& depth) {
    py::object selfObject = py::cast(&self, py::return_value_policy::reference);

    if (py::isinstance<py::str>(data)) {
        return self.byData(data.cast<std::string>(), depth);
    }

    // bool must be checked before int (Python bool is an int subclass)
    if (py::isinstance<py::bool_>(data)) {
        return tryByDataMethod(selfObject, "by_data_bool", data, depth);
    }

    if (py::isinstance<py::int_>(data)) {
        if (auto n = tryByDataMethod(selfObject, "by_data_int64", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_uint64", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_int32", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_uint32", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_int16", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_uint16", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_int8", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_uint8", data, depth)) return n;
        return nullptr;
    }

    if (py::isinstance<py::float_>(data)) {
        if (auto n = tryByDataMethod(selfObject, "by_data_double", data, depth)) return n;
        if (auto n = tryByDataMethod(selfObject, "by_data_float", data, depth)) return n;
        return nullptr;
    }

    // NumPy scalars (and size-1 arrays) can usually be unwrapped through item().
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dispatchByData(self, scalar, depth);
            }
        } catch (const py::error_already_set&) {
            // Fall through to unified type error below.
        }
    }

    throw py::type_error(
        "by_data: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}


std::shared_ptr<Node> dispatchByAnd(
    Navigation& self,
    const std::string& name,
    const std::string& type,
    const py::object& data,
    const size_t& depth) {
    py::object selfObject = py::cast(&self, py::return_value_policy::reference);

    if (py::isinstance<py::str>(data)) {
        return self.byAnd(name, type, data.cast<std::string>(), depth);
    }

    // bool must be checked before int (Python bool is an int subclass)
    if (py::isinstance<py::bool_>(data)) {
        return tryByAndMethod(selfObject, "by_and_bool",name,type,data, depth);
    }

    if (py::isinstance<py::int_>(data)) {
        if (auto n = tryByAndMethod(selfObject, "by_and_int64", name,type,data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_uint64", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_int32", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_uint32", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_int16", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_uint16", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_int8", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_uint8", name, type, data, depth)) return n;
        return nullptr;
    }

    if (py::isinstance<py::float_>(data)) {
        if (auto n = tryByAndMethod(selfObject, "by_and_double", name, type, data, depth)) return n;
        if (auto n = tryByAndMethod(selfObject, "by_and_float", name, type, data, depth)) return n;
        return nullptr;
    }

    // NumPy scalars (and size-1 arrays) can usually be unwrapped through item().
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dispatchByAnd(self, name, type, scalar, depth);
            }
        } catch (const py::error_already_set&) {
            // Fall through to unified type error below.
        }
    }

    throw py::type_error(
        "by_and: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}


std::vector<std::shared_ptr<Node>> dispatchAllByData(
    Navigation& self,
    const py::object& data,
    const size_t& depth) {
    py::object selfObject = py::cast(&self, py::return_value_policy::reference);

    if (py::isinstance<py::str>(data)) {
        return self.allByData(data.cast<std::string>(), depth);
    }

    std::vector<std::shared_ptr<Node>> output;

    // bool must be checked before int (Python bool is an int subclass)
    if (py::isinstance<py::bool_>(data)) {
        if (tryAllByDataMethod(selfObject, "all_by_data_bool", data, depth, output)) return output;
        return {};
    }

    if (py::isinstance<py::int_>(data)) {
        if (tryAllByDataMethod(selfObject, "all_by_data_int64", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_uint64", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_int32", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_uint32", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_int16", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_uint16", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_int8", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_uint8", data, depth, output)) return output;
        return {};
    }

    if (py::isinstance<py::float_>(data)) {
        if (tryAllByDataMethod(selfObject, "all_by_data_double", data, depth, output)) return output;
        if (tryAllByDataMethod(selfObject, "all_by_data_float", data, depth, output)) return output;
        return {};
    }

    // NumPy scalars (and size-1 arrays) can usually be unwrapped through item().
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dispatchAllByData(self, scalar, depth);
            }
        } catch (const py::error_already_set&) {
            // Fall through to unified type error below.
        }
    }

    throw py::type_error(
        "all_by_data: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}

std::vector<std::shared_ptr<Node>> dispatchAllByAnd(
    Navigation& self,
    const std::string& name,
    const std::string& type,
    const py::object& data,
    const size_t& depth) {
    py::object selfObject = py::cast(&self, py::return_value_policy::reference);

    if (py::isinstance<py::str>(data)) {
        return self.allByAnd(name, type, data.cast<std::string>(), depth);
    }

    std::vector<std::shared_ptr<Node>> output;

    // bool must be checked before int (Python bool is an int subclass)
    if (py::isinstance<py::bool_>(data)) {
        if (tryAllByAndMethod(selfObject, "all_by_and_bool", name, type, data, depth, output)) return output;
        return {};
    }

    if (py::isinstance<py::int_>(data)) {
        if (tryAllByAndMethod(selfObject, "all_by_and_int64", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_uint64", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_int32", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_uint32", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_int16", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_uint16", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_int8", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_uint8", name, type, data, depth, output)) return output;
        return {};
    }

    if (py::isinstance<py::float_>(data)) {
        if (tryAllByAndMethod(selfObject, "all_by_and_double", name, type, data, depth, output)) return output;
        if (tryAllByAndMethod(selfObject, "all_by_and_float", name, type, data, depth, output)) return output;
        return {};
    }

    // NumPy scalars (and size-1 arrays) can usually be unwrapped through item().
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dispatchAllByAnd(self, name, type, scalar, depth);
            }
        } catch (const py::error_already_set&) {
            // Fall through to unified type error below.
        }
    }

    throw py::type_error(
        "all_by_and: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}

} // namespace

void bindNavigation(py::module_ &m) {
    auto navigation = py::class_<Navigation>(m, "Navigation");

    navigation
        .def("child_by_name", &Navigation::childByName, "Get child node by name")
        .def("by_name",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byName),
             "get node by exact name recursively",
             py::arg("name"), py::arg("depth")=100)
        .def("all_by_name",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByName),
             "get all nodes by exact name recursively",
             py::arg("name"), py::arg("depth")=100)
        .def("by_name_regex",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byNameRegex),
             "get node by regex-pattern name recursively",
             py::arg("name_pattern"), py::arg("depth")=100)
        .def("all_by_name_regex",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByNameRegex),
             "get all nodes by regex-pattern name recursively",
             py::arg("name_pattern"), py::arg("depth")=100)
        .def("by_name_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byNameGlob),
             "get node by glob-pattern name recursively",
             py::arg("name_pattern"), py::arg("depth")=100)
        .def("all_by_name_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByNameGlob),
             "get all nodes by glob-pattern name recursively",
             py::arg("name_pattern"), py::arg("depth")=100)
        .def("child_by_type", &Navigation::childByType, "Get child node by type")
        .def("by_type",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byType),
             "get node by exact type recursively",
             py::arg("type"), py::arg("depth")=100)
        .def("all_by_type",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByType),
             "get all nodes by exact type recursively",
             py::arg("type"), py::arg("depth")=100)
        .def("by_type_regex",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byTypeRegex),
             "get node by regex-pattern type recursively",
             py::arg("type_pattern"), py::arg("depth")=100)
        .def("all_by_type_regex",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByTypeRegex),
             "get all nodes by regex-pattern type recursively",
             py::arg("type_pattern"), py::arg("depth")=100)
        .def("by_type_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byTypeGlob),
             "get node by glob-pattern type recursively",
             py::arg("type_pattern"), py::arg("depth")=100)
        .def("all_by_type_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByTypeGlob),
             "get all nodes by glob-pattern type recursively",
             py::arg("type_pattern"), py::arg("depth")=100)
        .def("child_by_data",
             [](Navigation& self, const py::object& data) {
                 return dispatchChildByData(self, data);
             },
             "Get child node by data (string or scalar)",
             py::arg("data"))
        .def("by_data",
             [](Navigation& self, const py::object& data, const size_t& depth) {
                 return dispatchByData(self, data, depth);
             },
             "get node by exact data recursively (string or scalar)",
             py::arg("data"), py::arg("depth")=100)
        .def("by_data_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::byDataGlob),
             "get node by glob-pattern data recursively",
             py::arg("data_pattern"), py::arg("depth")=100)
        .def("all_by_data",
             [](Navigation& self, const py::object& data, const size_t& depth) {
                 return dispatchAllByData(self, data, depth);
             },
             "get all nodes by exact data recursively (string or scalar)",
             py::arg("data"), py::arg("depth")=100)
        .def("all_by_data_glob",
             py::overload_cast<const std::string&, const size_t&>(&Navigation::allByDataGlob),
             "get all nodes by glob-pattern data recursively",
             py::arg("data_pattern"), py::arg("depth")=100)
        .def("by_and",
             [](Navigation& self, const std::string& name, const std::string& type,
                    const py::object& data, const size_t& depth) {
                 return dispatchByAnd(self, name, type, data, depth);
             },
             "get node by and condition using name, type and data recursively (string or scalar)",
             py::arg("name")=std::string(""),
             py::arg("type")=std::string(""),
             py::arg("data")=std::string(""),
             py::arg("depth")=100)
        .def("all_by_and",
             [](Navigation& self, const std::string& name, const std::string& type,
                    const py::object& data, const size_t& depth) {
                 return dispatchAllByAnd(self, name, type, data, depth);
             },
             "get all nodes by and condition using name, type and data recursively (string or scalar)",
             py::arg("name")=std::string(""),
             py::arg("type")=std::string(""),
             py::arg("data")=std::string(""),
             py::arg("depth")=100);

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

    utils::bindClassMethodForScalarTypes(
        navigation,
        "by_data",
        []<typename T>(utils::TypeTag<T>) {
            return [](Navigation& self, T data, const size_t& depth) {
                return self.template byData<T>(data, depth);
            };
        },
        py::arg("data"),
        py::arg("depth")=100,
        "Get node by scalar data recursively (typed overload)");

    utils::bindClassMethodForScalarTypes(
        navigation,
        "all_by_data",
        []<typename T>(utils::TypeTag<T>) {
            return [](Navigation& self, T data, const size_t& depth) {
                return self.template allByData<T>(data, depth);
            };
        },
        py::arg("data"),
        py::arg("depth")=100,
        "Get all nodes by scalar data recursively (typed overload)");

    utils::bindClassMethodForScalarTypes(
        navigation,
        "by_and",
        []<typename T>(utils::TypeTag<T>) {
            return [](Navigation& self,
                      const std::string& name,
                      const std::string& type,
                      T data,
                      const size_t& depth) {
                return self.template byAnd<T>(name, type, data, depth);
            };
        },
        py::arg("name")=std::string(""),
        py::arg("type")=std::string(""),
        py::arg("data"),
        py::arg("depth")=100,
        "Get node by name/type and scalar data recursively (typed overload)");

    utils::bindClassMethodForScalarTypes(
        navigation,
        "all_by_and",
        []<typename T>(utils::TypeTag<T>) {
            return [](Navigation& self,
                      const std::string& name,
                      const std::string& type,
                      T data,
                      const size_t& depth) {
                return self.template allByAnd<T>(name, type, data, depth);
            };
        },
        py::arg("name")=std::string(""),
        py::arg("type")=std::string(""),
        py::arg("data"),
        py::arg("depth")=100,
        "Get all nodes by name/type and scalar data recursively (typed overload)");

}
