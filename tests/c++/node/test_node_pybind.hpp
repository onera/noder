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
    sm.def("test_positionAmongSiblings_null", &test_positionAmongSiblings_null);
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
    sm.def("test_positionAmongSiblings", &test_positionAmongSiblings);
}

# endif