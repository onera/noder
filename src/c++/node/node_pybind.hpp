# ifndef NODE_PYBIND_HPP
# define NODE_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "node/node.hpp"
# include "node/node_factory.hpp"

// node_factory_pybind.hpp (or inside bindNode)

static std::shared_ptr<Node> new_node(
    const std::string& name,
    const std::string& type,
    py::object data = py::bool_(false),
    std::shared_ptr<Node> parent = nullptr)
{
    if (data.is_none()) {
        return newNode<bool>(name, type, false, parent);
    }

    // Python str -> std::string
    if (py::isinstance<py::str>(data)) {
        return newNode<std::string>(name, type, data.cast<std::string>(), parent);
    }

    // bool (must be checked before int, because bool is an int subclass in Python)
    if (py::isinstance<py::bool_>(data)) {
        return newNode<bool>(name, type, data.cast<bool>(), parent);
    }

    // integers
    if (py::isinstance<py::int_>(data)) {
        // pick one canonical integer type for your API
        return newNode<int64_t>(name, type, data.cast<int64_t>(), parent);
    }

    // floats
    if (py::isinstance<py::float_>(data)) {
        return newNode<double>(name, type, data.cast<double>(), parent);
    }

    // numpy array -> your Array (assuming you have Array(py::array) ctor)
    if (py::isinstance<py::array>(data)) {
        Array a(data.cast<py::array>());
        return newNode<Array>(name, type, a, parent);
    }

    // already an Array object (if Array is bound)
    if (py::isinstance<Array>(data)) {
        return newNode<Array>(name, type, data.cast<Array>(), parent);
    }

    throw py::type_error("newNode: unsupported data type: " + py::str(data.get_type()).cast<std::string>());
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
        
        .def("set_data", [](Node &node, std::shared_ptr<Data> d) {
            node.setData(std::move(d));  // Forward the shared_ptr<Data> correctly
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