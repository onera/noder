# ifndef NODE_PYBIND_HPP
# define NODE_PYBIND_HPP

# include <pybind11/pybind11.h>
# include <pybind11/numpy.h>
# include <pybind11/stl.h>

# include "array/array.hpp"
# include "node/node.hpp"
# include "node/node_group_pybind.hpp"
# include "node/node_factory.hpp"

namespace {

std::shared_ptr<Data> dataFromPyObject(const py::object& data, const char* context) {
    if (data.is_none()) {
        return std::make_shared<Array>();
    }

    if (py::isinstance<Data>(data)) {
        return data.cast<std::shared_ptr<Data>>();
    }

    if (py::isinstance<py::str>(data)) {
        return std::make_shared<Array>(data.cast<std::string>());
    }

    // bool before int because bool is an int subclass in Python
    if (py::isinstance<py::bool_>(data)) {
        return std::make_shared<Array>(data.cast<bool>());
    }

    if (py::isinstance<py::int_>(data)) {
        return std::make_shared<Array>(data.cast<int64_t>());
    }

    if (py::isinstance<py::float_>(data)) {
        return std::make_shared<Array>(data.cast<double>());
    }

    if (py::isinstance<py::array>(data)) {
        return std::make_shared<Array>(data.cast<py::array>());
    }

    if (py::isinstance<py::list>(data) || py::isinstance<py::tuple>(data)) {
        try {
            py::module_ np = py::module_::import("numpy");
            py::array arrayData = np.attr("asarray")(data).cast<py::array>();
            return std::make_shared<Array>(arrayData);
        } catch (const py::error_already_set&) {
            throw py::type_error(
                std::string(context) + ": list/tuple could not be converted to a NumPy array");
        }
    }

    // NumPy scalars (and 0d/size-1 arrays) can often be converted through item()
    if (py::hasattr(data, "item")) {
        try {
            py::object scalar = data.attr("item")();
            if (scalar.ptr() != data.ptr()) {
                return dataFromPyObject(scalar, context);
            }
        } catch (const py::error_already_set&) {
            // Fall through to final type error.
        }
    }

    throw py::type_error(
        std::string(context) +
        ": unsupported data type: " + py::str(data.get_type()).cast<std::string>());
}

ParameterValue::DictEntries parameterEntriesFromPyMapping(const py::handle& mapping, const char* context);

ParameterValue parameterValueFromPyObject(const py::handle& value, const char* context) {
    py::object objectValue = py::reinterpret_borrow<py::object>(value);

    if (objectValue.is_none() || PyCallable_Check(objectValue.ptr())) {
        return ParameterValue::makeNull();
    }

    if (py::isinstance<Node>(objectValue)) {
        return ParameterValue::makeNull();
    }

    if (py::isinstance<py::dict>(objectValue)) {
        return ParameterValue::makeDict(parameterEntriesFromPyMapping(objectValue, context));
    }

    if (py::isinstance<py::list>(objectValue) || py::isinstance<py::tuple>(objectValue)) {
        py::sequence sequenceValue = objectValue.cast<py::sequence>();
        const size_t sequenceSize = py::len(sequenceValue);
        if (sequenceSize == 0) {
            return ParameterValue::makeNull();
        }

        py::object firstElement = py::reinterpret_borrow<py::object>(sequenceValue[0]);
        if (firstElement.is_none() || py::isinstance<Node>(firstElement)) {
            return ParameterValue::makeNull();
        }

        if (py::isinstance<py::dict>(firstElement)) {
            ParameterValue::ListEntries entries;
            entries.reserve(sequenceSize);
            for (size_t i = 0; i < sequenceSize; ++i) {
                py::object item = py::reinterpret_borrow<py::object>(sequenceValue[i]);
                if (!py::isinstance<py::dict>(item)) {
                    throw py::value_error(std::string(context) + ": expected dict in list of dicts");
                }
                entries.push_back(ParameterValue::makeDict(parameterEntriesFromPyMapping(item, context)));
            }
            return ParameterValue::makeList(std::move(entries));
        }
    }

    return ParameterValue::makeData(dataFromPyObject(objectValue, context));
}

ParameterValue::DictEntries parameterEntriesFromPyMapping(const py::handle& mapping, const char* context) {
    if (!py::isinstance<py::dict>(mapping)) {
        throw py::type_error(std::string(context) + ": expected dict-like mapping");
    }

    py::dict mappingDict = py::reinterpret_borrow<py::dict>(mapping);
    ParameterValue::DictEntries entries;
    entries.reserve(mappingDict.size());

    for (const auto& item : mappingDict) {
        if (!py::isinstance<py::str>(item.first)) {
            throw py::type_error(std::string(context) + ": parameter names must be strings");
        }
        const std::string parameterName = item.first.cast<std::string>();
        entries.emplace_back(parameterName, parameterValueFromPyObject(item.second, context));
    }
    return entries;
}

py::object pyObjectFromParameterValue(const ParameterValue& value, bool transformNumpyScalars) {
    switch (value.kind) {
        case ParameterValue::Kind::Null:
            return py::none();

        case ParameterValue::Kind::Data: {
            if (!value.data || value.data->isNone()) {
                return py::none();
            }

            auto arrayData = std::dynamic_pointer_cast<Array>(value.data);
            if (!arrayData) {
                return py::cast(value.data);
            }

            py::array output = arrayData->getPyArray();
            if (transformNumpyScalars && output.size() == 1) {
                try {
                    return output.attr("item")();
                } catch (const py::error_already_set&) {
                    // Keep array output when item() is not applicable.
                }
            }
            return output;
        }

        case ParameterValue::Kind::Dict: {
            py::dict output;
            for (const auto& [key, childValue] : value.dictEntries) {
                output[py::str(key)] = pyObjectFromParameterValue(childValue, transformNumpyScalars);
            }
            return output;
        }

        case ParameterValue::Kind::List: {
            py::list output;
            for (const auto& childValue : value.listEntries) {
                output.append(pyObjectFromParameterValue(childValue, transformNumpyScalars));
            }
            return output;
        }
    }

    throw py::value_error("get_parameters: unsupported ParameterValue kind");
}

} // namespace

static std::shared_ptr<Node> new_node(
    const std::string& name,
    const std::string& type,
    py::object data = py::none(),
    std::shared_ptr<Node> parent = nullptr)
{
    auto node = std::make_shared<Node>(name, type);
    node->setData(dataFromPyObject(data, "new_node"));
    if (parent) {
        node->attachTo(parent);
    }
    return node;
}


void bindNode(py::module_ &m) {

    auto nodeClass = py::class_<Node, std::shared_ptr<Node>>(m, "Node")

        .def(py::init<const std::string&, const std::string&>(), "Node constructor",
             py::arg("name"), py::arg("type")="DataArray_t")

        .def("pick", &Node::pick)  

        .def("name", &Node::name)
        .def("set_name", py::overload_cast<const std::string&>(&Node::setName))

        .def("data", [](const Node &node) -> py::object {
            std::shared_ptr<Data> data = node.dataPtr();
            if (!data || data->isNone()) {
                return py::none();
            }
            return py::cast(data);  // Cast to correct type (`Array`, `ParallelArray`)
        })
        
        .def("set_data", [](Node &node, const py::object& d) {
            node.setData(dataFromPyObject(d, "set_data"));
        })
        .def(
            "set_parameters",
            [](Node& node,
               const std::string& containerName,
               const std::string& containerType,
               const std::string& parameterType,
               py::kwargs parameters) {
                ParameterValue::DictEntries entries;
                entries.reserve(parameters.size());
                for (const auto& item : parameters) {
                    if (!py::isinstance<py::str>(item.first)) {
                        throw py::type_error("set_parameters: parameter names must be strings");
                    }
                    const std::string parameterName = item.first.cast<std::string>();
                    entries.emplace_back(parameterName, parameterValueFromPyObject(item.second, "set_parameters"));
                }
                return node.setParameters(containerName, entries, containerType, parameterType);
            },
            py::arg("container_name"),
            py::arg("container_type") = "UserDefinedData_t",
            py::arg("parameter_type") = "DataArray_t")
        .def(
            "get_parameters",
            [](const Node& node, const std::string& containerName, bool transformNumpyScalars) {
                const ParameterValue parameters = node.getParameters(containerName);
                return pyObjectFromParameterValue(parameters, transformNumpyScalars);
            },
            py::arg("container_name"),
            py::arg("transform_numpy_scalars") = false)
                
        .def("children", &Node::children)
        .def("type", &Node::type)
        .def("set_type", py::overload_cast<const std::string&>(&Node::setType))
        .def("has_link_target", &Node::hasLinkTarget)
        .def("link_target_file", &Node::linkTargetFile)
        .def("link_target_path", &Node::linkTargetPath)
        .def("get_links", &Node::getLinks)
        .def("set_link_target", &Node::setLinkTarget,
             py::arg("target_file"), py::arg("target_path"))
        .def("clear_link_target", &Node::clearLinkTarget)
        .def("reload_node_data", &Node::reloadNodeData, py::arg("filename"))
        .def("save_this_node_only", &Node::saveThisNodeOnly,
             py::arg("filename"), py::arg("backend")="hdf5")
        .def("parent", [](const Node &self) -> std::shared_ptr<Node> {
            return self.parent().lock();
        }, "Returns the parent Node or None if no parent exists.")
        .def("root", &Node::root)
        .def("level", &Node::level)
        .def("position", &Node::position)
        .def("detach", &Node::detach)
        .def("attach_to", &Node::attachTo, "attach this node to another",
             py::arg("node"),
             py::arg("position")=-1,
             py::arg("override_sibling_by_name")=true)
        .def("add_child", &Node::addChild,
             py::arg("node"),
             py::arg("override_sibling_by_name")=true,
             py::arg("position")=-1)
        .def("has_children", &Node::hasChildren)
        .def("siblings", &Node::siblings, py::arg("include_myself")=true)
        .def("has_siblings", &Node::hasSiblings)
        .def("get_children_names", &Node::getChildrenNames)
        .def("add_children", &Node::addChildren,
             py::arg("nodes"),
             py::arg("override_sibling_by_name")=true)
        .def("swap", &Node::swap, py::arg("node"))
        .def("copy", &Node::copy, py::arg("deep")=false)
        .def("get_at_path", &Node::getAtPath, py::arg("path"), py::arg("path_is_relative")=false)
        .def("merge", &Node::merge, py::arg("node"))
        .def("path", &Node::path)
        #ifdef ENABLE_HDF5_IO
        .def("write", &Node::write)
        #endif
        .def("descendants", &Node::descendants)


        .def("__str__", &Node::__str__)
        .def("print_tree", &Node::printTree, "print node in tree format",
             py::arg("max_depth")=9999,  py::arg("highlighted_path")=std::string(""),
             py::arg("depth")=0, py::arg("last_pos")=false, py::arg("markers")=std::string(""))

        ;

    bindNodeGroup(m, nodeClass);

    m.def(
        "new_node",
        &new_node,
        py::arg("name") = "",
        py::arg("type") = "",
        py::arg("data") = py::none(),
        py::arg("parent") = nullptr
    );

}

# endif
