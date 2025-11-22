# ifndef NODE_PYBIND_HPP
# define NODE_PYBIND_HPP
# include "node/node.hpp"

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

}

# endif