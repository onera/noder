# ifndef TEST_NODE_PYBIND_HPP
# define TEST_NODE_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_node.hpp"

void bindTestsOfNode(py::module_ &m) {

    py::module_ sm = m.def_submodule("node");
    sm.def("test_init", &test_init);
    sm.def("test_name", &test_name);
    sm.def("test_setName", &test_setName);
    sm.def("test_type", &test_type);
    sm.def("test_setType", &test_setType);
    sm.def("test_binding_setNameAndTypeFromPython", &test_binding_setNameAndTypeFromPython);
    sm.def("test_noData", &test_noData);
    sm.def("test_children_empty", &test_children_empty);
    sm.def("test_parent_empty", &test_parent_empty);
    sm.def("test_root_itself", &test_root_itself);
    sm.def("test_level_0", &test_level_0);
    sm.def("test_position_null", &test_position_null);
    sm.def("test_getPath_itself", &test_getPath_itself);
    sm.def("test_attachTo", &test_attachTo);
    sm.def("test_attachTo_multiple_levels", &test_attachTo_multiple_levels);
    sm.def("test_addChild", &test_addChild);
    sm.def("test_addChildAsPointer", &test_addChildAsPointer);
    sm.def("test_detach_0", &test_detach_0);
    sm.def("test_detach_1", &test_detach_1);
    sm.def("test_detach_2", &test_detach_2);
    sm.def("test_detach_3", &test_detach_3);
    sm.def("test_delete_multiple_descendants", &test_delete_multiple_descendants);
    sm.def("test_getPath", &test_getPath);
    sm.def("test_root", &test_root);
    sm.def("test_level", &test_level);
    sm.def("test_printTree", &test_printTree);
    sm.def("test_children", &test_children);
    sm.def("test_binding_addChildrenFromPython", &test_binding_addChildrenFromPython);
    sm.def("test_position", &test_position);
    sm.def("test_hasChildren", &test_hasChildren);
    sm.def("test_siblings", &test_siblings);
    sm.def("test_hasSiblings", &test_hasSiblings);
    sm.def("test_getChildrenNames", &test_getChildrenNames);
    sm.def("test_addChildren", &test_addChildren);
    sm.def("test_overrideSiblingByName_attachTo", &test_overrideSiblingByName_attachTo);
    sm.def("test_overrideSiblingByName_addChild", &test_overrideSiblingByName_addChild);
    sm.def("test_overrideSiblingByName_addChildren", &test_overrideSiblingByName_addChildren);
    sm.def("test_swap", &test_swap);
    sm.def("test_copy", &test_copy);
    sm.def("test_getAtPath", &test_getAtPath);
    sm.def("test_getLinks", &test_getLinks);
#ifdef ENABLE_HDF5_IO
    sm.def("test_reloadNodeData", &test_reloadNodeData,
           py::arg("filename") = std::string("test_reload_node_data.cgns"));
    sm.def("test_saveThisNodeOnly", &test_saveThisNodeOnly,
           py::arg("filename") = std::string("test_save_this_node_only.cgns"));
#endif
    sm.def("test_merge", &test_merge);
}

# endif
