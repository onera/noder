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
# include <algorithm>
# include <stdexcept>
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

    /** @brief Allocate an array of shape @p shape filled with @p fill_value. */
    template <typename T>
    Array full(const std::vector<size_t>& shape, T fill_value, const char order = 'C');

    /** @brief Allocate an uninitialized array. */
    template <typename T>
    Array empty(const std::vector<size_t>& shape, const char order = 'C');

    /** @brief Compute C-order strides in bytes for @p shape. */
    template <typename T>
    std::vector<size_t> computeStridesInOrderC(const std::vector<size_t>& shape);

    /** @brief Compute Fortran-order strides in bytes for @p shape. */
    template <typename T>
    std::vector<size_t> computeStridesInOrderF(const std::vector<size_t>& shape);

    /** @brief Allocate a zero-initialized array. */
    template <typename T>
    Array zeros(const std::vector<size_t>& shape, const char order = 'C');

    /** @brief Allocate a one-initialized array. */
    template <typename T>
    Array ones(const std::vector<size_t>& shape, const char order = 'C');


}

// Template implementations kept in header to avoid cross-TU symbol issues.
namespace arrayfactory {

    template <typename T>
    Array full(const std::vector<size_t>& shape, T fill_value, const char order) {
        Array array = empty<T>(shape, order);
        T* data = array.getPointerOfModifiableDataFast<T>();
        std::fill(data, data + array.size(), fill_value);
        return array;
    }

    template <typename T>
    Array empty(const std::vector<size_t>& shape, const char order) {
        std::vector<size_t> strides;
        if (order == 'C') {
            strides = computeStridesInOrderC<T>(shape);
        } else if (order == 'F') {
            strides = computeStridesInOrderF<T>(shape);
        } else {
            throw std::invalid_argument("order must be either 'C' or 'F'");
        }
        py::array pyarray = py::array(py::dtype::of<T>(), shape, strides);
        return Array(pyarray);
    }

    template <typename T>
    std::vector<size_t> computeStridesInOrderC(const std::vector<size_t>& shape) {
        size_t stride = sizeof(T);
        const size_t dims = shape.size();
        std::vector<size_t> strides(shape.size());
        for (size_t j = 0; j < dims; ++j) {
            const size_t dim = dims - 1 - j;
            strides[dim] = stride;
            stride *= shape[dim];
        }
        return strides;
    }

    template <typename T>
    std::vector<size_t> computeStridesInOrderF(const std::vector<size_t>& shape) {
        size_t stride = sizeof(T);
        std::vector<size_t> strides(shape.size());
        for (size_t i = 0; i < shape.size(); ++i) {
            strides[i] = stride;
            stride *= shape[i];
        }
        return strides;
    }

    template <typename T>
    Array zeros(const std::vector<size_t>& shape, const char order) {
        return full(shape, static_cast<T>(0), order);
    }

    template <typename T>
    Array ones(const std::vector<size_t>& shape, const char order) {
        return full(shape, static_cast<T>(1), order);
    }
}

# endif 
