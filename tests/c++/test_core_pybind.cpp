# include <pybind11/pybind11.h>
# include <pybind11/stl.h>

# include "io/test_io.hpp"
# include "io/test_yaml_io.hpp"

# include "array/test_array_pybind.hpp"
# include "node/test_node_pybind.hpp"
# include "node/test_node_factory_pybind.hpp"
# include "data/data_factory.hpp"
# include "node/test_data_pybind.hpp"
# include "node/test_navigation_pybind.hpp"
# include "node/test_node_group_pybind.hpp"
# include "cgns/test_zone_pybind.hpp"
# include "learn_pybind11/test_learn_pybind11_pybind.hpp"
# include "utils/test_utils_pybind.hpp"

PYBIND11_MODULE(tests, m) {

    ensureFactoryInitialized(); // Not really working

    py::module_ io_m = m.def_submodule("io");
    // TODO redesign io test pybindings
    # ifdef ENABLE_HDF5_IO
    io_m.def("test_write_nodes", &test_io::test_write_nodes, "test write a cgns file",
                   py::arg("filename")=std::string("test.cgns"));
    io_m.def("test_read", &test_io::test_read, "test read a cgns file",
                   py::arg("tmp_filename")=std::string("test_read.cgns"));
    io_m.def("test_write_link_nodes", &test_io::test_write_link_nodes, "test write cgns links",
                   py::arg("filename")=std::string("test_links.cgns"));
    io_m.def("test_read_links", &test_io::test_read_links, "test read cgns links",
                   py::arg("tmp_filename")=std::string("test_read_links.cgns"));
    io_m.def("list_root_links", &test_io::list_root_links, "list physical HDF5 root links",
                   py::arg("filename"));

    # endif 
    io_m.def("test_write_yaml_nodes", &test_io::test_write_yaml_nodes, "test write a yaml file",
                   py::arg("filename")=std::string("test.yaml"));
    io_m.def("test_read_yaml", &test_io::test_read_yaml, "test read a yaml file",
                   py::arg("filename")=std::string("test_read.yaml"));
    io_m.def("test_write_yaml_link_nodes", &test_io::test_write_yaml_link_nodes, "test write yaml links",
                   py::arg("filename")=std::string("test_links.yaml"));
    io_m.def("test_read_yaml_links", &test_io::test_read_yaml_links, "test read yaml links",
                   py::arg("filename")=std::string("test_read_links.yaml"));
    io_m.def("test_read_yaml_cgns_tree", &test_io::test_read_yaml_cgns_tree, "test read yaml cgns tree",
                   py::arg("filename")=std::string("test_tree.yaml"));

    bindTestsOfArray(m);
    bindTestsOfNode(m);
    bindTestsOfNodeFactory(m);
    bindTestsOfData(m);
    bindTestsOfNavigation(m);
    bindTestsOfNodeGroup(m);
    bindTestsOfZone(m);
    bindTestsOfUtils(m);
    
    bindLearningTestsOfPyBind11(m);

}
