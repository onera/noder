# include <pybind11/pybind11.h>

# ifdef ENABLE_HDF5_IO
# include "io/test_io.hpp"
# endif

# include "array/test_array_pybind.hpp"
# include "node/test_node_pybind.hpp"
# include "node/test_node_factory_pybind.hpp"
# include "data/data_factory.hpp"
# include "node/test_data_pybind.hpp"
# include "node/test_navigation_pybind.hpp"
# include "node/test_node_group_pybind.hpp"
# include "learn_pybind11/test_learn_pybind11_pybind.hpp"

PYBIND11_MODULE(tests, m) {

    ensureFactoryInitialized(); // Not really working

    // TODO redesign io test pybindings
    # ifdef ENABLE_HDF5_IO
    py::module_ io_m = m.def_submodule("io");
    io_m.def("test_write_nodes", &test_io::test_write_nodes, "test write a cgns file",
                   py::arg("filename")=std::string("test.cgns"));
    io_m.def("test_read", &test_io::test_read, "test read a cgns file",
                   py::arg("tmp_filename")=std::string("test_read.cgns"));
    io_m.def("test_write_link_nodes", &test_io::test_write_link_nodes, "test write cgns links",
                   py::arg("filename")=std::string("test_links.cgns"));
    io_m.def("test_read_links", &test_io::test_read_links, "test read cgns links",
                   py::arg("tmp_filename")=std::string("test_read_links.cgns"));

    # endif 

    bindTestsOfArray(m);
    bindTestsOfNode(m);
    bindTestsOfNodeFactory(m);
    bindTestsOfData(m);
    bindTestsOfNavigation(m);
    bindTestsOfNodeGroup(m);
    
    bindLearningTestsOfPyBind11(m);

}
