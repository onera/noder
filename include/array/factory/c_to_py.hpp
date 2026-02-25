#ifndef C_TO_PY_HPP
#define C_TO_PY_HPP

#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <pybind11/numpy.h>

# include "array/array.hpp"

class Array;

namespace arrayfactory {

    namespace py = pybind11;

   /*
        Implementation is done in header, so that it forces template
        instantiation at compile-time, hence avoiding undefined symbol errors at
        execution.
        
                                    /!\ BEWARE /!\
        Using py::none() as last argument tells pybind11 that numpy array 
        does NOT own its data ("arrayInC" has ownership). This may be dangerous if 
        arrayInC goes out of scope or if it reallocates memory, which will provoke
        py::array to have a dangling pointer.
        This is the best approach in general :
         1. initialize numpy arrays owning data (either in C++ or Python),
            by copying from auxiliar array if convenient. 
         2. manipulate arrays in-place by getting pointer.
         3. re-assign new numpy value if array shall be extended
    */
    /**
     * @brief Convert contiguous 1D C memory to Array.
     * @param arrayInC Pointer to first element.
     * @param nbOfItems Number of elements.
     * @param copy If false, Array references caller-managed memory.
     */
    template <typename T>
    Array toArray1D(T* arrayInC, const size_t& nbOfItems, const bool& copy=true) {
        ssize_t signedNbOfItems = static_cast<ssize_t>(nbOfItems);
        if ( copy ) {
            return Array(py::array_t<T>(signedNbOfItems, arrayInC));
        } else {
            return Array(py::array(py::dtype::of<T>(), signedNbOfItems, arrayInC, py::none()));
        }
    }

    /**
     * @brief Convert std::array to 1D Array.
     * @param arrayInC Source fixed-size array.
     * @param copy If false, returned Array references std::array storage.
     */
    template <typename T, std::size_t N>
    Array toArray1D(const std::array<T, N>& arrayInC, const bool& copy=true) {

        if (copy) {
            return Array(py::array_t<T>(N, arrayInC.data()));
        } else {
            return Array(py::array(py::dtype::of<T>(), N, arrayInC.data(), py::none()));
        }
    }


    /**
     * @brief Convert std::vector to 1D Array.
     * @param arrayInC Source vector.
     * @param copy If false, storage is shared when contiguous and supported.
     */
    template <typename T>
    Array toArray1D(const std::vector<T>& arrayInC, const bool& copy = true) {
        if constexpr (std::is_same_v<T, bool>) {
            std::vector<uint8_t> boolArray(arrayInC.begin(), arrayInC.end());

            if (copy) {
                ssize_t arraySize = static_cast<ssize_t>(boolArray.size());
                auto pyarray = py::array_t<uint8_t>(arraySize, boolArray.data());
                return Array(pyarray);
            } else {
                throw py::attribute_error("Cannot share memory for std::vector<bool>, use int8_t or copy=true.");
            }

        } else {
            // Normal case (vector<int>, vector<float>, etc.)
            ssize_t arraySize = static_cast<ssize_t>(arrayInC.size());
            if (copy) {
                return Array(py::array_t<T>(arraySize, arrayInC.data()));
            } else {
                return Array(py::array(py::dtype::of<T>(), arraySize, arrayInC.data(), py::none()));
            }
        }
    }


    /** @brief Convert contiguous pointer storage to 2D Array. */
    template <typename T>
    Array toArray2D(T* arrayInC, const size_t& rows, const size_t& cols, const bool& copy = false) {
        ssize_t sRows = static_cast<ssize_t>(rows);
        ssize_t sCols = static_cast<ssize_t>(cols);
        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({sRows, sCols});
            T* buffer = static_cast<T*>(pyarray.mutable_data());
            std::copy(arrayInC, arrayInC + rows * cols, buffer);
            return Array(pyarray);
        } else {
            // Share memory directly
            return Array(py::array(py::dtype::of<T>(), {sRows, sCols}, arrayInC, py::none()));
        }
    }

    /** @brief Convert fixed-size nested std::array to 2D Array. */
    template <typename T, std::size_t ROWS, std::size_t COLS>
    Array toArray2D(const std::array<std::array<T, COLS>, ROWS>& arrayInC, const bool& copy = false) {
        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({ROWS, COLS});
            T* buffer = static_cast<T*>(pyarray.mutable_data());
            for (std::size_t i = 0; i < ROWS; ++i) {
                std::copy(arrayInC[i].begin(), arrayInC[i].end(), buffer + i * COLS);
            }
            return Array(pyarray);
        } else {
            // Share memory directly
            return Array(py::array(py::dtype::of<T>(), {ROWS, COLS}, &arrayInC[0][0], py::none()));
        }
    }

    /**
     * @brief Convert vector-of-vectors to 2D Array.
     * @note Non-copy path is unsupported because nested vectors are non-contiguous.
     */
    template <typename T>
    Array toArray2D(const std::vector<std::vector<T>>& arrayInC, const bool& copy = false) {
        size_t rows = arrayInC.size();
        size_t cols = arrayInC.empty() ? 0 : arrayInC[0].size();

        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({rows, cols});
            T* buffer = static_cast<T*>(pyarray.mutable_data());
            for (size_t i = 0; i < rows; ++i) {
                std::copy(arrayInC[i].begin(), arrayInC[i].end(), buffer + i * cols);
            }
            return Array(pyarray);
        } else {
            // NumPy requires contiguous memory for 2D arrays, so no shared memory here.
            throw py::attribute_error("cannot share memory for \
                std::vector<std::vector> due to non-contiguity, please set copy=true");
        }
    }

    /** @brief Convert contiguous pointer storage to 3D Array. */
    template <typename T>
    Array toArray3D(T* arrayInC, size_t rows, size_t cols, size_t depth, const bool& copy = false) {
        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({rows, cols, depth});
            T* buffer = static_cast<T*>(pyarray.mutable_data());
            std::copy(arrayInC, arrayInC + rows * cols * depth, buffer);
            return Array(pyarray);
        } else {
            // Share memory directly
            return Array(py::array(py::dtype::of<T>(), {rows, cols, depth}, arrayInC, py::none()));
        }
    }

    /** @brief Convert fixed-size nested std::array to 3D Array. */
    template <typename T, size_t ROWS, size_t COLS, size_t DEPTH>
    Array toArray3D(const std::array<std::array<std::array<T, DEPTH>, COLS>, ROWS>& arrayInC,
                        const bool& copy = false) {
        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({ROWS, COLS, DEPTH});
            T* buffer = static_cast<T*>(pyarray.mutable_data());

            for (std::size_t i = 0; i < ROWS; ++i) {
                for (std::size_t j = 0; j < COLS; ++j) {
                    std::copy(arrayInC[i][j].begin(), arrayInC[i][j].end(), buffer + (i * COLS + j) * DEPTH);
                }
            }
            return Array(pyarray);
        } else {
            // Share memory directly
            return Array(py::array( py::dtype::of<T>(),
                {ROWS, COLS, DEPTH},        // Shape
                {sizeof(T) * COLS * DEPTH,  // Strides for rows
                 sizeof(T) * DEPTH,         // Strides for columns
                 sizeof(T)},                // Strides for depth
                &arrayInC[0][0][0],         // Pointer to data
                py::none()                  // Ensures no ownership transfer
                ));
        }
    }

    /**
     * @brief Convert vector-of-vector-of-vector to 3D Array.
     * @note Non-copy path is unsupported because nested vectors are non-contiguous.
     */
    template <typename T>
    Array toArray3D(const std::vector<std::vector<std::vector<T>>>& arrayInC, const bool& copy = false) {
        size_t rows = arrayInC.size();
        size_t cols = rows > 0 ? arrayInC[0].size() : 0;
        size_t depth = (cols > 0 && rows > 0) ? arrayInC[0][0].size() : 0;

        if (copy) {
            // Create a deep copy
            py::array_t<T> pyarray({rows, cols, depth});
            T* buffer = static_cast<T*>(pyarray.mutable_data());
            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {
                    std::copy(arrayInC[i][j].begin(), arrayInC[i][j].end(), buffer + (i * cols + j) * depth);
                }
            }
            return Array(pyarray);
        } else {
            // NumPy requires contiguous memory for 3D arrays, so no shared memory here.
            throw py::attribute_error("Cannot share memory for non-contiguous std::vector<std::vector<std::vector>>. Set copy=true.");
        }
    }


}

#endif
