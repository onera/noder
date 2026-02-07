# ifndef ARRAYS_VECTORS_HPP
# define ARRAYS_VECTORS_HPP

# include <iostream>
# include <array>
# include <vector>
# include <string>
# include <cstdint>
# include <pybind11/numpy.h>

# include "array/array.hpp"

namespace arrayfactory {

    template <typename T>
    Array uniformFromStep(const double& start,
                 const double& stop,
                 const double& step = 1);

    template <typename T>
    Array uniformFromCount(const double& start,
                   const double& stop,
                   const size_t& num,
                   const bool& endpoint = true);

} 

# endif