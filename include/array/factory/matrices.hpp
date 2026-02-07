# ifndef ARRAYS_MATRICES_HPP
# define ARRAYS_MATRICES_HPP

# include <iostream>
# include <array>
# include <vector>
# include <string>
# include <cstdint>
# include <tuple>
# include <utility>
# include <type_traits>
# include <pybind11/numpy.h>

# include "array/array.hpp"


/* 
    For mixed Python/C++ projects, it is possible to directly call numpy 
    array makers using pybind11 capacity to call Python interpreter, e.g.:
    
    py::array pyarray = py::module::import("numpy").attr("zeros")(py::make_tuple(3, 4));

    However, calling Python from C++ is more costly than directly calling pure
    C++ functions, so it is preferred to use the array makers proposed in this
    module when suitable.
*/

namespace arrayfactory {

    namespace py = pybind11;

    template <typename T>
    Array full(const std::vector<size_t>& shape, T fill_value, const char order = 'C');

    template <typename T>
    Array empty(const std::vector<size_t>& shape, const char order = 'C');

    template <typename T>
    std::vector<size_t> computeStridesInOrderC(const std::vector<size_t>& shape);

    template <typename T>
    std::vector<size_t> computeStridesInOrderF(const std::vector<size_t>& shape);

    template <typename T>
    Array zeros(const std::vector<size_t>& shape, const char order = 'C');

    template <typename T>
    Array ones(const std::vector<size_t>& shape, const char order = 'C');


}

# endif 