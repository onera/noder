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

class PYBIND11_EXPORT Array : public Data {

private:

    py::array pyArray;
    size_t _dimensions;
    size_t _size;
    std::vector<size_t> _shape;
    std::vector<size_t> _strides;

    class Assertions;
    // Mark as mutable for lazy initialization.
    mutable std::shared_ptr<Assertions> _must; 

public:
    Array();
    Array(const py::array& other);
    Array(const std::string& string);
    Array(const Array& other);

    Assertions& must() const;

    /*
        methods
    */

    std::shared_ptr<Data> clone() const override;

    bool hasString() const override;
    bool isNone() const override;
    bool isScalar() const override;
    bool isContiguous() const;
    bool isContiguousInStyleC() const;
    bool isContiguousInStyleFortran() const;

    template <typename T> [[gnu::used]]
    bool hasDataOfType() const;

    void print(const size_t& maxChars=80) const;
    std::string getPrintString(const size_t& maxChars=80) const;

    /*
        modifiers
    */
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

    template <typename T> [[gnu::used]]
    Array& operator=(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& operator+=(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& operator-=(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& operator*=(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& operator/=(const T& scalar);

    // TODO implement basic math operations
    //  floor
    //  ceiling
    //  between

    /* 
        accessors
    */
   
    Array operator[](const size_t flatIndex);

    template <typename T> [[gnu::used]]
    T& getItemAtIndex(const size_t& index);

    template <typename T> [[gnu::used]]
    const T& getItemAtIndex(const size_t& index) const;

    template <typename T> [[gnu::used]]
    T* getPointerOfDataSafely() ;

    template <typename T> [[gnu::used]]
    T* getPointerOfModifiableDataFast();

    template <typename T> [[gnu::used]]
    const T* getPointerOfReadOnlyDataFast() const;

    template <typename T, ssize_t DIMS> [[gnu::used]]
    py::detail::unchecked_reference<T, DIMS> getAccessorOfReadOnlyData() const;

    template <typename T, ssize_t DIMS> [[gnu::used]]
    py::detail::unchecked_mutable_reference<T, DIMS> getAccessorOfModifiableData();

    size_t getFlatIndex(const std::vector<size_t>& indices) const;

    std::string extractString() const override;

    // required for Python coupling since binding cannot directly access members
    py::array getPyArray() const { return this->pyArray; };


    /*
        consultation of array properties
    */

    size_t dimensions() const {
        return this->_dimensions;
    }

    size_t size() const {
        return this->_size;
    }

    std::vector<size_t> shape() const {
        return this->_shape;
    }

    std::vector<size_t> strides() const {
        return this->_strides;
    }

    std::string info() const override;
    std::string shortInfo() const override;

    /* 
        comparisons
    */

    bool operator==(const Array& other) const;
    bool operator==(const int8_t& scalar) const;
    bool operator==(const int16_t& scalar) const;
    bool operator==(const int32_t& scalar) const;
    bool operator==(const int64_t& scalar) const;
    bool operator==(const uint8_t& scalar) const;
    bool operator==(const uint16_t& scalar) const;
    bool operator==(const uint32_t& scalar) const;
    bool operator==(const uint64_t& scalar) const;
    bool operator==(const float& scalar) const;
    bool operator==(const double& scalar) const;
    bool operator==(const bool& scalar) const;
    bool operator==(const std::string& string) const;

    bool operator!=(const Array& other) const;
    bool operator!=(const int8_t& scalar) const;
    bool operator!=(const int16_t& scalar) const;
    bool operator!=(const int32_t& scalar) const;
    bool operator!=(const int64_t& scalar) const;
    bool operator!=(const uint8_t& scalar) const;
    bool operator!=(const uint16_t& scalar) const;
    bool operator!=(const uint32_t& scalar) const;
    bool operator!=(const uint64_t& scalar) const;
    bool operator!=(const float& scalar) const;
    bool operator!=(const double& scalar) const;
    bool operator!=(const bool& scalar) const;
    bool operator!=(const std::string& otherString) const;

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

    template <typename T> [[gnu::used]]
    std::string getNumericalArrayPrint() const;

    /*
        modifiers
    */

    void setArrayMembersAsNull();
    void setArrayMembersUsing(const py::array& pyarray);

    template <typename T> [[gnu::used]]
    Array& setElementsAs(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& setElementsFrom(const Array& other);
    template <typename T> [[gnu::used]]
    Array& setElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& setElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& setElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& setElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T> [[gnu::used]]
    Array& increaseElementsBy(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& increaseElementsFrom(const Array& other);
    template <typename T> [[gnu::used]]
    Array& increaseElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& increaseElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& increaseElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& increaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T> [[gnu::used]]
    Array& decreaseElementsFrom(const Array& other);
    template <typename T> [[gnu::used]]
    Array& decreaseElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& decreaseElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& decreaseElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& decreaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T> [[gnu::used]]
    Array& multiplyElementsBy(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& multiplyElementsFrom(const Array& other);
    template <typename T> [[gnu::used]]
    Array& multiplyElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& multiplyElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& multiplyElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& multiplyElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    template <typename T> [[gnu::used]]
    Array& divideElementsBy(const T& scalar);

    template <typename T> [[gnu::used]]
    Array& divideElementsFrom(const Array& other);
    template <typename T> [[gnu::used]]
    Array& divideElementsOfThisContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& divideElementsOfThisContiguousUsingNonContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& divideElementsOfThisNonContiguousUsingContiguous(const Array& other);
    template <typename T> [[gnu::used]]
    Array& divideElementsOfThisNonContiguousUsingNonContiguous(const Array& other);


    /*
        accessors
    */

    template <typename T> [[gnu::used]]
    Array getItemAsArrayAtIndex(const size_t& flatIndex);

    template <typename T> [[gnu::used]]
    size_t getOffsetFromFlatIndex(size_t flatIndex) const;

    size_t getFlatIndexOfArrayInStyleC(const std::vector<size_t>& indices) const;
    size_t getFlatIndexOfArrayInStyleFortran(const std::vector<size_t>& indices) const;

    std::string extractStringOfKindU() const;
    std::string extractStringOfKindS() const;


    /*
        comparisons
    */

    template <typename T> [[gnu::used]]
    bool hasAllItemsEqualToThoseIn(const Array& other) const;

    template <typename T> [[gnu::used]]
    bool hasAllItemsEqualTo(const T& scalar) const;

    template <typename T> [[gnu::used]]
    bool hasAtLeastOneItemDifferentToThoseIn(const Array& other) const;

    template <typename T> [[gnu::used]]
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

# endif