# ifndef ARRAY_HPP
# define ARRAY_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <array>
# include <algorithm>
# include <cstdint>
# include <variant>
# include <regex>
# include <utility>
# include <sstream>
# include <memory>
# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

# include "utils/compat.hpp"
# include "utils/comparator.hpp"
# include "utils/template_instantiator.hpp"
# include "utils/template_binder.hpp"
# include "utils/string.hpp"

# include "data/data.hpp"

namespace py = pybind11;

/**
 * @brief NumPy-backed concrete Data implementation.
 *
 * Array wraps a `py::array` and exposes typed access, shape/stride metadata,
 * scalar and string conversions, and in-place arithmetic helpers.
 */
class PYBIND11_EXPORT Array : public Data {

private:

    py::array pyArray;
    size_t _dimensions;
    size_t _size;
    std::vector<size_t> _shape;
    std::vector<size_t> _strides;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    class Assertions;
    // Mark as mutable for lazy initialization.
    mutable std::shared_ptr<Assertions> _must; 
#endif

public:
    /** @name Constructors
     *  @{
     */
    Array();
    Array(const py::array& other);
    Array(const std::string& string);
    Array(const char* string);
    Array(const Array& other);

    Array(int8_t scalar);
    Array(int16_t scalar);
    Array(int32_t scalar);
    Array(int64_t scalar);
    Array(uint8_t scalar);
    Array(uint16_t scalar);
    Array(uint32_t scalar);
    Array(uint64_t scalar);
    Array(float scalar);
    Array(double scalar);
    Array(bool scalar);
    /** @} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    /** @brief Access runtime assertion helpers. */
    Assertions& must() const;
#endif

    /*
        methods
    */

    /** @brief Create a shallow clone of this payload. */
    std::shared_ptr<Data> clone() const override;
    /**
     * @brief Copy this payload.
     * @param deep When true, underlying array memory is duplicated.
     */
    std::shared_ptr<Data> copy(bool deep=false) const override;

    /** @brief True when dtype is string-like. */
    bool hasString() const override;
    /** @brief True when payload is empty/none-like. */
    bool isNone() const override;
    /** @brief True when payload is a scalar numeric value. */
    bool isScalar() const override;
    /** @brief True when memory is contiguous in C or Fortran order. */
    bool isContiguous() const;
    /** @brief True when memory is C-order contiguous. */
    bool isContiguousInStyleC() const;
    /** @brief True when memory is Fortran-order contiguous. */
    bool isContiguousInStyleFortran() const;

    /**
     * @brief Check whether array dtype matches @p T.
     * @tparam T Candidate scalar type.
     */
    template <typename T>
    bool hasDataOfType() const;

    /** @brief Print array preview to stdout. */
    void print(const size_t& maxChars=80) const;
    /** @brief Return array preview string. */
    std::string getPrintString(const size_t& maxChars=80) const;

    /*
        modifiers
    */
    /** @brief Assign by sharing wrapped py::array metadata/buffer. */
    Array& operator=(const Array& other) {
        if (this != &other) {
            this->pyArray = other.pyArray;
            this->_dimensions = other._dimensions;
            this->_size = other._size;
            this->_shape = other._shape;
            this->_strides = other._strides;
        }
        return *this;
    }

    /** @name In-place scalar operations
     *  @{
     */
    template <typename T>
    Array& operator=(const T& scalar);

    template <typename T>
    Array& operator+=(const T& scalar);

    template <typename T>
    Array& operator-=(const T& scalar);

    template <typename T>
    Array& operator*=(const T& scalar);

    template <typename T>
    Array& operator/=(const T& scalar);
    /** @} */

    // TODO implement basic math operations
    //  floor
    //  ceiling
    //  between

    /* 
        accessors
    */
   
    /** @brief Return element at flat index as one-element Array. */
    Array operator[](const size_t flatIndex);

    /** @brief Typed mutable element access by flat index. */
    template <typename T>
    T& getItemAtIndex(const size_t& index);

    /** @brief Typed read-only element access by flat index. */
    template <typename T>
    const T& getItemAtIndex(const size_t& index) const;

    /** @brief Typed mutable pointer with runtime checks. */
    template <typename T>
    T* getPointerOfDataSafely() ;

    /** @brief Fast mutable pointer access (requires dtype correctness). */
    template <typename T>
    T* getPointerOfModifiableDataFast();

    /** @brief Fast read-only pointer access (requires dtype correctness). */
    template <typename T>
    const T* getPointerOfReadOnlyDataFast() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    template <typename T, ssize_t DIMS>
    PYBIND11_EXPORT
    py::detail::unchecked_reference<T, DIMS> getAccessorOfReadOnlyData() const;

    template <typename T, ssize_t DIMS>
    PYBIND11_EXPORT
    py::detail::unchecked_mutable_reference<T, DIMS> getAccessorOfModifiableData();
#endif

    /** @brief Convert multidimensional indices to flat offset. */
    size_t getFlatIndex(const std::vector<size_t>& indices) const;

    /** @brief Decode string payload as UTF-8 text. */
    std::string extractString() const override;

    // required for Python coupling since binding cannot directly access members
    /** @brief Expose wrapped `py::array` value. */
    py::array getPyArray() const { return this->pyArray; };


    /*
        consultation of array properties
    */

    /** @brief Number of array dimensions. */
    size_t dimensions() const {
        return this->_dimensions;
    }

    /** @brief Total number of elements. */
    size_t size() const {
        return this->_size;
    }

    /** @brief Shape vector. */
    std::vector<size_t> shape() const {
        return this->_shape;
    }

    /** @brief Strides (bytes per dimension). */
    std::vector<size_t> strides() const {
        return this->_strides;
    }

    /** @brief Detailed metadata and payload summary. */
    std::string info() const override;
    /** @brief Compact metadata summary. */
    std::string shortInfo() const override;

    /* 
        comparisons
    */

    /** @name Comparison operators
     *  @{
     */
    bool operator==(const Array& other) const;
    bool operator==(const int8_t& scalar) const override;
    bool operator==(const int16_t& scalar) const override;
    bool operator==(const int32_t& scalar) const override;
    bool operator==(const int64_t& scalar) const override;
    bool operator==(const uint8_t& scalar) const override;
    bool operator==(const uint16_t& scalar) const override;
    bool operator==(const uint32_t& scalar) const override;
    bool operator==(const uint64_t& scalar) const override;
    bool operator==(const float& scalar) const override;
    bool operator==(const double& scalar) const override;
    bool operator==(const bool& scalar) const override;
    bool operator==(const std::string& string) const;
    bool operator==(const char* otherString) const;

    bool operator!=(const Array& other) const;
    bool operator!=(const int8_t& scalar) const override;
    bool operator!=(const int16_t& scalar) const override;
    bool operator!=(const int32_t& scalar) const override;
    bool operator!=(const int64_t& scalar) const override;
    bool operator!=(const uint8_t& scalar) const override;
    bool operator!=(const uint16_t& scalar) const override;
    bool operator!=(const uint32_t& scalar) const override;
    bool operator!=(const uint64_t& scalar) const override;
    bool operator!=(const float& scalar) const override;
    bool operator!=(const double& scalar) const override;
    bool operator!=(const bool& scalar) const override;
    bool operator!=(const std::string& otherString) const;
    bool operator!=(const char* otherString) const;
    /** @} */

    /*
        TODO math operators
    */
    
    // basic:
    //   (public) + add
    //   (public) - substract
    //   (public) * multiply
    //   (public) / divide
    //   (module) max
    //   (module) min
    //   (module) argmax
    //   (module) argmin

    // boolean masks:
    //   (public) getMaskOfValuesEqualTo
    //   (public) getMaskOfValuesInTheClosedInterval
    //   (public) getMaskOfValuesInTheOpenInterval
    //   (public) getMaskOfValuesGreaterThan
    //   (public) getMaskOfValuesGreaterOrEqualThan
    //   (public) getMaskOfValuesLessThan
    //   (public) getMaskOfValuesLessOrEqualThan

    // algebraic:
    //   (public) transpose (2D) 
    //   (public) dot product    
    //   (public) cross product  

    // statistic:
    //   (module) mean
    //   (module) rms
    //   (module) std
    //   (module) sum
    //   (module) cumsum


private:

    std::string getNumericalArrayPrintDispatchingByType() const;

    template <typename T>
    std::string getNumericalArrayPrint() const;

    /*
        modifiers
    */

    void setArrayMembersAsNull();
    void setArrayMembersUsing(const py::array& pyarray);

    template <typename T>
    Array& setElementsAs(const T& scalar);

    template <typename T>
    Array& setElementsFrom(const Array& other);
    template <typename T>
    Array& setElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& setElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T>
    Array& setElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& setElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T>
    Array& increaseElementsBy(const T& scalar);

    template <typename T>
    Array& increaseElementsFrom(const Array& other);
    template <typename T>
    Array& increaseElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& increaseElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T>
    Array& increaseElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& increaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T>
    Array& decreaseElementsFrom(const Array& other);
    template <typename T>
    Array& decreaseElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& decreaseElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T>
    Array& decreaseElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& decreaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T>
    Array& multiplyElementsBy(const T& scalar);

    template <typename T>
    Array& multiplyElementsFrom(const Array& other);
    template <typename T>
    Array& multiplyElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& multiplyElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T>
    Array& multiplyElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& multiplyElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T>
    Array& divideElementsBy(const T& scalar);

    template <typename T>
    Array& divideElementsFrom(const Array& other);
    template <typename T>
    Array& divideElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& divideElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T>
    Array& divideElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T>
    Array& divideElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    /*
        accessors
    */

    template <typename T>
    Array getItemAsArrayAtIndex(const size_t& flatIndex);

    template <typename T>
    size_t getOffsetFromFlatIndex(size_t flatIndex) const;

    size_t getFlatIndexOfArrayInStyleC(const std::vector<size_t>& indices) const;
    size_t getFlatIndexOfArrayInStyleFortran(const std::vector<size_t>& indices) const;

    std::string extractStringOfKindU() const;
    std::string extractStringOfKindS() const;


    /*
        comparisons
    */

    template <typename T>
    bool hasAllItemsEqualToThoseIn(const Array& other) const;

    template <typename T>
    bool hasAllItemsEqualTo(const T& scalar) const;

    template <typename T>
    bool hasAtLeastOneItemDifferentToThoseIn(const Array& other) const;

    template <typename T>
    bool hasAtLeastOneItemDifferentTo(const T& scalar) const;

    bool hasSameStringAsThatIn(const Array& other) const;
    bool hasDifferentStringAsThatIn(const Array& other) const;

    bool hasSameStringAs(const std::string& otherString) const;
    bool hasDifferentStringTo(const std::string& otherString) const;


};

# include "array/factory/vectors.hpp"
# include "array/factory/matrices.hpp"
# include "array/factory/strings.hpp"
# include "array/assertions.hpp"

template <typename T>
inline bool Array::hasDataOfType() const {
    auto dtype = this->pyArray.dtype();
    return dtype.is(py::dtype::of<T>());
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <typename T, ssize_t DIMS>
inline py::detail::unchecked_reference<T, DIMS> Array::getAccessorOfReadOnlyData() const {
    if (!this->hasDataOfType<T>()) {
        std::string msg = "Wrong requested type ";
        msg += utils::getTypeName<T>();
        throw py::type_error(msg);
    }
    if (static_cast<size_t>(DIMS) != this->dimensions()) {
        throw py::type_error("Expected dimensions: " + std::to_string(DIMS) +
                             ", but got: " + std::to_string(this->dimensions()));
    }
    return this->pyArray.unchecked<T, static_cast<size_t>(DIMS)>();
}

template <typename T, ssize_t DIMS>
inline py::detail::unchecked_mutable_reference<T, DIMS> Array::getAccessorOfModifiableData() {
    if (!this->hasDataOfType<T>()) {
        std::string msg = "Wrong requested type ";
        msg += utils::getTypeName<T>();
        throw py::type_error(msg);
    }
    if (static_cast<size_t>(DIMS) != this->dimensions()) {
        throw py::type_error("Expected dimensions: " + std::to_string(DIMS) +
                             ", but got: " + std::to_string(this->dimensions()));
    }
    return this->pyArray.mutable_unchecked<T, DIMS>();
}
#endif

# endif
