# include "node/navigation.hpp"
# include "node/node.hpp"

void bindNavigation(py::module_ &m) {
    py::class_<Navigation>(m, "Navigation")
        .def("childByName", &Navigation::childByName, "Get child node by name")
        .def("byName",
          py::overload_cast<const std::string&, const int&>(&Navigation::byName),
          "get node by exact name recursively",
          py::arg("name"), py::arg("depth")=100)
        .def("byNamePattern",
          py::overload_cast<const std::string&, const int&>(&Navigation::byNamePattern),
          "get node by regex-pattern name recursively",
          py::arg("name_pattern"), py::arg("depth")=100)
        .def("childByType", &Navigation::childByType, "Get child node by type")
        .def("byType",
          py::overload_cast<const std::string&, const int&>(&Navigation::byType),
          "get node by exact type recursively",
          py::arg("type"), py::arg("depth")=100)
        .def("byTypePattern",
          py::overload_cast<const std::string&, const int&>(&Navigation::byTypePattern),
          "get node by regex-pattern type recursively",
          py::arg("type_pattern"), py::arg("depth")=100);

}
