#ifndef ARRAYS_MATRICES_HPP
#define ARRAYS_MATRICES_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "array/array.hpp"

namespace arrayfactory {

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

    namespace detail {
        inline size_t checkedByteCount(const std::vector<size_t>& shape, size_t itemsize) {
            size_t count = 1;
            for (size_t extent : shape) {
                if (extent != 0 && count > std::numeric_limits<size_t>::max() / extent) {
                    throw std::overflow_error("arrayfactory::empty: shape product overflow");
                }
                count *= extent;
            }
            if (count != 0 && itemsize > std::numeric_limits<size_t>::max() / count) {
                throw std::overflow_error("arrayfactory::empty: byte-size overflow");
            }
            return count * itemsize;
        }

        inline std::shared_ptr<void> allocateBytes(size_t byteCount) {
            if (byteCount == 0) {
                return nullptr;
            }
            return std::shared_ptr<void>(new std::uint8_t[byteCount], [](void* ptr) {
                delete[] static_cast<std::uint8_t*>(ptr);
            });
        }
    } // namespace detail

} // namespace arrayfactory

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

        const size_t byteCount = detail::checkedByteCount(shape, sizeof(T));
        auto owner = detail::allocateBytes(byteCount);
        return Array(Array::typeIdFor<T>(), sizeof(T), owner.get(), shape, strides, owner);
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

#endif
