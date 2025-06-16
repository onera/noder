# include "data/data_pybind.hpp"
# include "array/array_pybind.hpp"
# include "data/data_factory.hpp"
# include "node/node_pybind.hpp"
# include "node/navigation_pybind.hpp"
# include "io/cgns/node_pycgns_converter_pybind.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/io.hpp"
#endif


PYBIND11_MODULE(core, m) {

    m.def("registerDefaultFactory",&registerDefaultFactory);

    #ifdef ENABLE_HDF5_IO
    // TODO redesign io pybindings
    py::module_ io_m = m.def_submodule("io");
    io_m.def("read", &io::read, "read a CGNS HDF5 file as Nodes");
    io_m.def("write_numpy", &io::write_numpy, "write a numpy",
             py::arg("array"), py::arg("filename"), py::arg("dataset_name")=std::string("numpy"));
    io_m.def("read_numpy", &io::read_numpy, "read a numpy",
             py::arg("filename"), py::arg("dataset_name")=std::string("numpy"), py::arg("order")=std::string("F"));
    #endif

    bindData(m);
    bindArray(m);
    bindNode(m);
    bindNavigation(m);
    bindNodePyCGNSConverter(m);

}

