# ifndef NODE_PYBIND_HPP
# define NODE_PYBIND_HPP

# include <pybind11/pybind11.h>
# include <pybind11/numpy.h>

# include "array/array.hpp"
# include "node/node.hpp"
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

} // namespace

static std::shared_ptr<Node> new_node(
    const std::string& name,
    const std::string& type,
    py::object data = py::bool_(false),
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

    py::class_<Node, std::shared_ptr<Node>>(m, "Node")

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
                
        .def("children", &Node::children)
        .def("type", &Node::type)
        .def("set_type", py::overload_cast<const std::string&>(&Node::setType))
        .def("parent", [](const Node &self) -> std::shared_ptr<Node> {
            return self.parent().lock();
        }, "Returns the parent Node or None if no parent exists.")
        .def("root", &Node::root)
        .def("level", &Node::level)
        .def("position", &Node::position)
        .def("detach", &Node::detach)
        .def("attach_to", &Node::attachTo)
        .def("add_child", &Node::addChild)
        .def("path", &Node::path)
        .def("write", &Node::write)
        .def("descendants", &Node::descendants)


        .def("__str__", &Node::__str__)
        .def("print_tree", &Node::printTree, "print node in tree format",
             py::arg("max_depth")=9999,  py::arg("highlighted_path")=std::string(""),
             py::arg("depth")=0, py::arg("last_pos")=false, py::arg("markers")=std::string(""))

        ;
    m.def(
        "new_node",
        &new_node,
        py::arg("name") = "",
        py::arg("type") = "",
        py::arg("data") = py::bool_(false),
        py::arg("parent") = nullptr
    );

}

# endif
