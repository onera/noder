# include "node/navigation.hpp"
# include "node/node.hpp"

void bindNavigation(py::module_ &m) {
    py::class_<Navigation>(m, "Navigation")
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
          py::arg("type_pattern"), py::arg("depth")=100);

}
