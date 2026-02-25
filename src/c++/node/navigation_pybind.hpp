# include "node/navigation.hpp"
# include "node/node.hpp"
# include <pybind11/numpy.h>
# include <pybind11/stl.h>
# include "utils/pybind_dispatch.hpp"
# include "utils/template_binder.hpp"

namespace {

std::shared_ptr<Node> dispatchChildByData(Navigation& self, const py::object& data) {
    if (py::isinstance<py::str>(data)) {
        return self.childByData(data.cast<std::string>());
    }

    if (py::isinstance<py::bool_>(data)) {
        return self.childByData(data.cast<bool>());
    }

    if (py::isinstance<py::int_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonIntegralDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template childByData<T>(value);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    if (py::isinstance<py::float_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonFloatingDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template childByData<T>(value);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    py::object scalar;
    if (utils::tryUnwrapPyScalarItem(data, scalar)) {
        return dispatchChildByData(self, scalar);
    }

    throw py::type_error(
        "child_by_data: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}

std::shared_ptr<Node> dispatchByData(
    Navigation& self,
    const py::object& data,
    const size_t& depth) {
    if (py::isinstance<py::str>(data)) {
        return self.byData(data.cast<std::string>(), depth);
    }

    if (py::isinstance<py::bool_>(data)) {
        return self.byData(data.cast<bool>(), depth);
    }

    if (py::isinstance<py::int_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonIntegralDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template byData<T>(value, depth);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    if (py::isinstance<py::float_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonFloatingDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template byData<T>(value, depth);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    py::object scalar;
    if (utils::tryUnwrapPyScalarItem(data, scalar)) {
        return dispatchByData(self, scalar, depth);
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
    if (py::isinstance<py::str>(data)) {
        return self.byAnd(name, type, data.cast<std::string>(), depth);
    }

    if (py::isinstance<py::bool_>(data)) {
        return self.byAnd(name, type, data.cast<bool>(), depth);
    }

    if (py::isinstance<py::int_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonIntegralDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template byAnd<T>(name, type, value, depth);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    if (py::isinstance<py::float_>(data)) {
        std::shared_ptr<Node> result = nullptr;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonFloatingDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template byAnd<T>(name, type, value, depth);
                return static_cast<bool>(result);
            });
        if (state.anyCast) {
            return result;
        }
    }

    py::object scalar;
    if (utils::tryUnwrapPyScalarItem(data, scalar)) {
        return dispatchByAnd(self, name, type, scalar, depth);
    }

    throw py::type_error(
        "by_and: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}


std::vector<std::shared_ptr<Node>> dispatchAllByData(
    Navigation& self,
    const py::object& data,
    const size_t& depth) {
    if (py::isinstance<py::str>(data)) {
        return self.allByData(data.cast<std::string>(), depth);
    }

    if (py::isinstance<py::bool_>(data)) {
        return self.allByData(data.cast<bool>(), depth);
    }

    if (py::isinstance<py::int_>(data)) {
        std::vector<std::shared_ptr<Node>> result;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonIntegralDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template allByData<T>(value, depth);
                return !result.empty();
            });
        if (state.anyCast) {
            return result;
        }
    }

    if (py::isinstance<py::float_>(data)) {
        std::vector<std::shared_ptr<Node>> result;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonFloatingDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template allByData<T>(value, depth);
                return !result.empty();
            });
        if (state.anyCast) {
            return result;
        }
    }

    py::object scalar;
    if (utils::tryUnwrapPyScalarItem(data, scalar)) {
        return dispatchAllByData(self, scalar, depth);
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
    if (py::isinstance<py::str>(data)) {
        return self.allByAnd(name, type, data.cast<std::string>(), depth);
    }

    if (py::isinstance<py::bool_>(data)) {
        return self.allByAnd(name, type, data.cast<bool>(), depth);
    }

    if (py::isinstance<py::int_>(data)) {
        std::vector<std::shared_ptr<Node>> result;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonIntegralDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template allByAnd<T>(name, type, value, depth);
                return !result.empty();
            });
        if (state.anyCast) {
            return result;
        }
    }

    if (py::isinstance<py::float_>(data)) {
        std::vector<std::shared_ptr<Node>> result;
        auto state = utils::dispatchByCastedTypeList(
            data,
            utils::PythonFloatingDispatchTypes{},
            [&]<typename T>(T value) {
                result = self.template allByAnd<T>(name, type, value, depth);
                return !result.empty();
            });
        if (state.anyCast) {
            return result;
        }
    }

    py::object scalar;
    if (utils::tryUnwrapPyScalarItem(data, scalar)) {
        return dispatchAllByAnd(self, name, type, scalar, depth);
    }

    throw py::type_error(
        "all_by_and: unsupported data type; expected str or scalar "
        "(bool/int/float, including NumPy scalar types).");
}

} // namespace

void bindNavigation(py::module_ &m) {
    auto navigation = py::class_<Navigation>(
        m,
        "Navigation",
        R"doc(
Tree-query helper attached to :py:class:`noder.core.Node` via :py:meth:`Node.pick`.

Navigation methods search descendants by name, type, data, or combined predicates.

See C++ counterpart: :ref:`cpp-navigation-class`.
)doc");

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
