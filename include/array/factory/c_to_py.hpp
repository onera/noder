#ifndef C_TO_PY_HPP
#define C_TO_PY_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "array/array.hpp"
#include "array/factory/matrices.hpp"

namespace arrayfactory {

    namespace detail {
        inline size_t computeSizeFromShape(const std::vector<size_t>& shape) {
            size_t count = 1;
            for (size_t extent : shape) {
                count *= extent;
            }
            return count;
        }

        template <typename T>
        Array copyFromRawBuffer(const T* arrayInC,
                                const std::vector<size_t>& shape,
                                const std::vector<size_t>& strides) {
            const size_t itemCount = computeSizeFromShape(shape);
            const size_t byteCount = itemCount * sizeof(T);
            std::shared_ptr<void> owner;
            if (byteCount != 0) {
                owner = std::shared_ptr<void>(new std::uint8_t[byteCount], [](void* ptr) {
                    delete[] static_cast<std::uint8_t*>(ptr);
                });
                std::memcpy(owner.get(), arrayInC, byteCount);
            }
            return Array(Array::typeIdFor<T>(), sizeof(T), owner.get(), shape, strides, owner);
        }
    } // namespace detail

    /**
     * @brief Convert contiguous 1D C memory to Array.
     * @param arrayInC Pointer to first element.
     * @param nbOfItems Number of elements.
     * @param copy If false, Array references caller-managed memory.
     */
    template <typename T>
    Array toArray1D(T* arrayInC, const size_t& nbOfItems, const bool& copy = true) {
        const std::vector<size_t> shape = {nbOfItems};
        const std::vector<size_t> strides = {sizeof(T)};
        if (copy) {
            return detail::copyFromRawBuffer(arrayInC, shape, strides);
        }
        return Array(Array::typeIdFor<T>(), sizeof(T), arrayInC, shape, strides);
    }

    /**
     * @brief Convert std::array to 1D Array.
     * @param arrayInC Source fixed-size array.
     * @param copy If false, returned Array references std::array storage.
     */
    template <typename T, std::size_t N>
    Array toArray1D(const std::array<T, N>& arrayInC, const bool& copy = true) {
        return toArray1D(const_cast<T*>(arrayInC.data()), N, copy);
    }

    /**
     * @brief Convert std::vector to 1D Array.
     * @param arrayInC Source vector.
     * @param copy If false, storage is shared when contiguous and supported.
     */
    template <typename T>
    Array toArray1D(const std::vector<T>& arrayInC, const bool& copy = true) {
        if constexpr (std::is_same_v<T, bool>) {
            if (!copy) {
                throw std::invalid_argument("Cannot share memory for std::vector<bool>, use int8_t or copy=true.");
            }
            std::vector<uint8_t> boolArray(arrayInC.begin(), arrayInC.end());
            return toArray1D(boolArray.data(), boolArray.size(), true);
        } else {
            return toArray1D(const_cast<T*>(arrayInC.data()), arrayInC.size(), copy);
        }
    }

    /** @brief Convert contiguous pointer storage to 2D Array. */
    template <typename T>
    Array toArray2D(T* arrayInC, const size_t& rows, const size_t& cols, const bool& copy = false) {
        const std::vector<size_t> shape = {rows, cols};
        const std::vector<size_t> strides = {sizeof(T) * cols, sizeof(T)};
        if (copy) {
            return detail::copyFromRawBuffer(arrayInC, shape, strides);
        }
        return Array(Array::typeIdFor<T>(), sizeof(T), arrayInC, shape, strides);
    }

    /** @brief Convert fixed-size nested std::array to 2D Array. */
    template <typename T, std::size_t ROWS, std::size_t COLS>
    Array toArray2D(const std::array<std::array<T, COLS>, ROWS>& arrayInC, const bool& copy = false) {
        return toArray2D(const_cast<T*>(&arrayInC[0][0]), ROWS, COLS, copy);
    }

    /**
     * @brief Convert vector-of-vectors to 2D Array.
     * @note Non-copy path is unsupported because nested vectors are non-contiguous.
     */
    template <typename T>
    Array toArray2D(const std::vector<std::vector<T>>& arrayInC, const bool& copy = false) {
        const size_t rows = arrayInC.size();
        const size_t cols = arrayInC.empty() ? 0 : arrayInC[0].size();

        if (!copy) {
            throw std::invalid_argument(
                "cannot share memory for std::vector<std::vector> due to non-contiguity, please set copy=true");
        }

        Array output = empty<T>({rows, cols}, 'C');
        T* buffer = output.getPointerOfModifiableDataFast<T>();
        for (size_t i = 0; i < rows; ++i) {
            std::copy(arrayInC[i].begin(), arrayInC[i].end(), buffer + i * cols);
        }
        return output;
    }

    /** @brief Convert contiguous pointer storage to 3D Array. */
    template <typename T>
    Array toArray3D(T* arrayInC, size_t rows, size_t cols, size_t depth, const bool& copy = false) {
        const std::vector<size_t> shape = {rows, cols, depth};
        const std::vector<size_t> strides = {sizeof(T) * cols * depth, sizeof(T) * depth, sizeof(T)};
        if (copy) {
            return detail::copyFromRawBuffer(arrayInC, shape, strides);
        }
        return Array(Array::typeIdFor<T>(), sizeof(T), arrayInC, shape, strides);
    }

    /** @brief Convert fixed-size nested std::array to 3D Array. */
    template <typename T, size_t ROWS, size_t COLS, size_t DEPTH>
    Array toArray3D(const std::array<std::array<std::array<T, DEPTH>, COLS>, ROWS>& arrayInC,
                    const bool& copy = false) {
        return toArray3D(const_cast<T*>(&arrayInC[0][0][0]), ROWS, COLS, DEPTH, copy);
    }

    /**
     * @brief Convert vector-of-vector-of-vector to 3D Array.
     * @note Non-copy path is unsupported because nested vectors are non-contiguous.
     */
    template <typename T>
    Array toArray3D(const std::vector<std::vector<std::vector<T>>>& arrayInC, const bool& copy = false) {
        const size_t rows = arrayInC.size();
        const size_t cols = rows > 0 ? arrayInC[0].size() : 0;
        const size_t depth = (cols > 0 && rows > 0) ? arrayInC[0][0].size() : 0;

        if (!copy) {
            throw std::invalid_argument(
                "Cannot share memory for non-contiguous std::vector<std::vector<std::vector>>. Set copy=true.");
        }

        Array output = empty<T>({rows, cols, depth}, 'C');
        T* buffer = output.getPointerOfModifiableDataFast<T>();
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::copy(arrayInC[i][j].begin(), arrayInC[i][j].end(), buffer + (i * cols + j) * depth);
            }
        }
        return output;
    }
}

#endif
