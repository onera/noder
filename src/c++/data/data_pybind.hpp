# include "data/data.hpp"

# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

namespace py = pybind11;

void bindData(py::module_ &m) {
    py::class_<Data, std::shared_ptr<Data>>(m, "Data")
    .def("hasString", &Data::hasString)
    .def("isNone", &Data::isNone)
    .def("isScalar", &Data::isScalar)
    .def("extractString", &Data::extractString);
}
