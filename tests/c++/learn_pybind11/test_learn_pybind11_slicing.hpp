# ifndef TEST_LEARN_PYBIND11_SLICING_HPP
# define TEST_LEARN_PYBIND11_SLICING_HPP

# include <iostream>
# include <string>
# include <vector>
# include <cstdint>
# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

namespace py = pybind11;

void test_slicingReferences();

void test_pointerAccess();

# endif