#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "utils/compat.hpp"
#include "utils/comparator.hpp"
#include "utils/template_instantiator.hpp"
#include "utils/string.hpp"

#include "data/data.hpp"

enum class ArrayTypeId {
    None,
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Bytes,
    Unicode
};

enum class ArrayOwnerKind {
    None,
    ManagedBuffer,
    PythonArray
};

struct ArrayDType {
    ArrayTypeId id = ArrayTypeId::None;
    size_t itemsize = 0;

    bool isString() const;
    bool isNumeric() const;
    char kind() const;
    std::string name() const;
};

/**
 * @brief Pure C++ concrete Data implementation backed by raw memory and metadata.
 *
 * Array stores typed n-dimensional buffers, including non-contiguous views.
 * Python/NumPy conversion is implemented in the pybind layer, not here.
 */
class Array : public Data {

private:
    std::shared_ptr<void> _owner;
    ArrayOwnerKind _ownerKind;
    std::uint8_t* _data;
    ArrayDType _dtype;
    size_t _dimensions;
    size_t _size;
    std::vector<size_t> _shape;
    std::vector<size_t> _strides;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    class Assertions;
    mutable std::shared_ptr<Assertions> _must;
#endif

public:
    /** @name Constructors
     *  @{
     */
    Array();
    Array(ArrayTypeId typeId,
          size_t itemsize,
          void* data,
          const std::vector<size_t>& shape,
          const std::vector<size_t>& strides,
          std::shared_ptr<void> owner = nullptr);
    Array(ArrayTypeId typeId,
          size_t itemsize,
          void* data,
          const std::vector<size_t>& shape,
          const std::vector<size_t>& strides,
          std::shared_ptr<void> owner,
          ArrayOwnerKind ownerKind);
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

    template <typename T>
    static constexpr ArrayTypeId typeIdFor();

    static Array bytesView(void* data,
                           size_t itemsize,
                           const std::vector<size_t>& shape,
                           const std::vector<size_t>& strides,
                           std::shared_ptr<void> owner = nullptr);
    static Array bytesView(void* data,
                           size_t itemsize,
                           const std::vector<size_t>& shape,
                           const std::vector<size_t>& strides,
                           std::shared_ptr<void> owner,
                           ArrayOwnerKind ownerKind);

    static Array unicodeView(void* data,
                             size_t itemsize,
                             const std::vector<size_t>& shape,
                             const std::vector<size_t>& strides,
                             std::shared_ptr<void> owner = nullptr);
    static Array unicodeView(void* data,
                             size_t itemsize,
                             const std::vector<size_t>& shape,
                             const std::vector<size_t>& strides,
                             std::shared_ptr<void> owner,
                             ArrayOwnerKind ownerKind);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    Assertions& must() const;
#endif

    std::shared_ptr<Data> clone() const override;
    std::shared_ptr<Data> copy(bool deep = false) const override;

    bool hasString() const override;
    bool isNone() const override;
    bool isScalar() const override;
    size_t dimensions() const override { return this->_dimensions; }
    size_t size() const override { return this->_size; }
    std::vector<size_t> shape() const override { return this->_shape; }
    std::string dtype() const override;
    std::shared_ptr<Data> full(
        const std::vector<size_t>& shape,
        double value,
        const std::string& dtypeName,
        char order = 'F') const override;
    std::shared_ptr<Data> ravel(const std::string& order = "K") const override;
    std::shared_ptr<Data> take(int64_t index, size_t axis) const override;
    int64_t itemAsInt64(const std::vector<size_t>& indices) const override;
    void setItemFromInt64(const std::vector<size_t>& indices, int64_t value) override;

    bool isContiguous() const;
    bool isContiguousInStyleC() const;
    bool isContiguousInStyleFortran() const;

    template <typename T>
    bool hasDataOfType() const;

    void print(const size_t& maxChars = 80) const;
    std::string getPrintString(const size_t& maxChars = 80) const;

    Array& operator=(const Array& other);

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

    Array operator[](size_t flatIndex);

    template <typename T>
    T& getItemAtIndex(const size_t& index);

    template <typename T>
    const T& getItemAtIndex(const size_t& index) const;

    template <typename T>
    T* getPointerOfDataSafely();

    template <typename T>
    T* getPointerOfModifiableDataFast();

    template <typename T>
    const T* getPointerOfReadOnlyDataFast() const;

    size_t getFlatIndex(const std::vector<size_t>& indices) const;

    std::string extractString() const override;

    std::vector<size_t> strides() const { return this->_strides; }

    std::string info() const override;
    std::string shortInfo() const override;

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

    ArrayTypeId typeId() const { return this->_dtype.id; }
    size_t itemsize() const { return this->_dtype.itemsize; }
    char dtypeKind() const { return this->_dtype.kind(); }
    const void* rawData() const { return this->_data; }
    void* rawData() { return this->_data; }
    const std::shared_ptr<void>& owner() const { return this->_owner; }
    ArrayOwnerKind ownerKind() const { return this->_ownerKind; }

private:
    static size_t computeSizeFromShape(const std::vector<size_t>& shape);

    template <typename T>
    static Array makeScalar(T scalar);

    std::string getNumericalArrayPrintDispatchingByType() const;

    template <typename T>
    std::string getNumericalArrayPrint() const;

    void setArrayMembersAsNull();
    void setArrayMembersUsing(ArrayTypeId typeId,
                              size_t itemsize,
                              void* data,
                              const std::vector<size_t>& shape,
                              const std::vector<size_t>& strides,
                              std::shared_ptr<void> owner,
                              ArrayOwnerKind ownerKind = ArrayOwnerKind::ManagedBuffer);

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

    template <typename T>
    Array getItemAsArrayAtIndex(const size_t& flatIndex);

    template <typename T>
    size_t getOffsetFromFlatIndex(size_t flatIndex) const;

    size_t getByteOffsetFromFlatIndex(size_t flatIndex) const;
    size_t getByteOffsetFromIndices(const std::vector<size_t>& indices) const;
    size_t getFlatIndexOfArrayInStyleC(const std::vector<size_t>& indices) const;
    size_t getFlatIndexOfArrayInStyleFortran(const std::vector<size_t>& indices) const;

    std::string extractStringOfKindU() const;
    std::string extractStringOfKindS() const;

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

#include "array/factory/vectors.hpp"
#include "array/factory/matrices.hpp"
#include "array/factory/strings.hpp"
#include "array/assertions.hpp"

inline bool ArrayDType::isString() const {
    return this->id == ArrayTypeId::Bytes || this->id == ArrayTypeId::Unicode;
}

inline bool ArrayDType::isNumeric() const {
    return !this->isString() && this->id != ArrayTypeId::None;
}

inline char ArrayDType::kind() const {
    switch (this->id) {
        case ArrayTypeId::Bool: return 'b';
        case ArrayTypeId::Int8:
        case ArrayTypeId::Int16:
        case ArrayTypeId::Int32:
        case ArrayTypeId::Int64: return 'i';
        case ArrayTypeId::UInt8:
        case ArrayTypeId::UInt16:
        case ArrayTypeId::UInt32:
        case ArrayTypeId::UInt64: return 'u';
        case ArrayTypeId::Float32:
        case ArrayTypeId::Float64: return 'f';
        case ArrayTypeId::Bytes: return 'S';
        case ArrayTypeId::Unicode: return 'U';
        case ArrayTypeId::None: return 'V';
    }
    return 'V';
}

inline std::string ArrayDType::name() const {
    switch (this->id) {
        case ArrayTypeId::Bool: return "bool";
        case ArrayTypeId::Int8: return "int8";
        case ArrayTypeId::Int16: return "int16";
        case ArrayTypeId::Int32: return "int32";
        case ArrayTypeId::Int64: return "int64";
        case ArrayTypeId::UInt8: return "uint8";
        case ArrayTypeId::UInt16: return "uint16";
        case ArrayTypeId::UInt32: return "uint32";
        case ArrayTypeId::UInt64: return "uint64";
        case ArrayTypeId::Float32: return "float32";
        case ArrayTypeId::Float64: return "float64";
        case ArrayTypeId::Bytes: return "bytes";
        case ArrayTypeId::Unicode: return "unicode";
        case ArrayTypeId::None: return "none";
    }
    return "none";
}

template <typename T>
inline constexpr ArrayTypeId Array::typeIdFor() {
    if constexpr (std::is_same_v<T, bool>) {
        return ArrayTypeId::Bool;
    } else if constexpr (std::is_same_v<T, int8_t>) {
        return ArrayTypeId::Int8;
    } else if constexpr (std::is_same_v<T, int16_t>) {
        return ArrayTypeId::Int16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return ArrayTypeId::Int32;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return ArrayTypeId::Int64;
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        return ArrayTypeId::UInt8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return ArrayTypeId::UInt16;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return ArrayTypeId::UInt32;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return ArrayTypeId::UInt64;
    } else if constexpr (std::is_same_v<T, float>) {
        return ArrayTypeId::Float32;
    } else if constexpr (std::is_same_v<T, double>) {
        return ArrayTypeId::Float64;
    } else {
        static_assert(!sizeof(T*), "Unsupported Array::typeIdFor<T>()");
    }
}

template <typename T>
inline bool Array::hasDataOfType() const {
    return this->_dtype.id == typeIdFor<T>();
}

#endif
