# ifndef TEST_LEARN_PYBIND11_PYBIND_HPP
# define TEST_LEARN_PYBIND11_PYBIND_HPP

# include "test_learn_pybind11_slicing.hpp"


void bindLearningTestsOfPyBind11(py::module_ &m) {

    py::module_ sm = m.def_submodule("learn_pybind11");
    
    sm.def("slicingReferences", &test_slicingReferences);
    sm.def("pointerAccess", &test_pointerAccess);
}

# endif