#ifndef IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_PYBIND_HPP
#define IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_PYBIND_HPP

#ifdef ENABLE_HDF5_IO

#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

inline void bindLazyHdf5Reader(py::module_& module) {
    py::class_<io::hdf5::cgns::LazyHdf5Reader,
               std::shared_ptr<io::hdf5::cgns::LazyHdf5Reader>>(
        module,
        "LazyHdf5Reader",
        R"doc(
Lazy metadata-first reader for CGNS/HDF5 files.

The returned root is a regular Node. Direct children and payloads are loaded
on demand, so ``loaded_children()`` can be used for paged terminal views.
When ``safe_mode`` is true, malformed HDF5/CGNS nodes are replaced by
``Corrupted_t`` marker nodes and reported through ``has_warnings()``.
)doc")
        .def(
            py::init<const std::string&, char, bool>(),
            py::arg("filename"),
            py::arg("order") = 'F',
            py::arg("safe_mode") = false)
        .def("root", &io::hdf5::cgns::LazyHdf5Reader::root)
        .def("filename", &io::hdf5::cgns::LazyHdf5Reader::filename)
        .def("order", &io::hdf5::cgns::LazyHdf5Reader::order)
        .def("close", &io::hdf5::cgns::LazyHdf5Reader::close)
        .def("is_open", &io::hdf5::cgns::LazyHdf5Reader::isOpen)
        .def("has_warnings", &io::hdf5::cgns::LazyHdf5Reader::hasWarnings)
        .def("safe_mode", &io::hdf5::cgns::LazyHdf5Reader::safeMode)
        .def(
            "ensure_children_loaded",
            &io::hdf5::cgns::LazyHdf5Reader::ensureChildrenLoaded,
            py::arg("node"),
            py::arg("minimum_children") = 0)
        .def(
            "ensure_data_loaded",
            &io::hdf5::cgns::LazyHdf5Reader::ensureDataLoaded,
            py::arg("node"));
}

#endif // ENABLE_HDF5_IO

#endif // IO_HDF5_LAZYCGNS_LAZY_HDF5_READER_PYBIND_HPP
