#include "array/array.hpp"
#include "array/assertions.hpp"
#include "array/factory/matrices.hpp"
#include "array/factory/strings.hpp"

#include <cctype>
#include <codecvt>
#include <cstring>
#include <limits>
#include <locale>
#include <stdexcept>

namespace {

char normalizeOrder(char order, const char* context) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(order)));
    if (normalized != 'C' && normalized != 'F') {
        throw std::invalid_argument(std::string(context) + ": order must be 'C' or 'F'");
    }
    return normalized;
}

size_t checkedProduct(const std::vector<size_t>& shape, const char* context) {
    size_t product = 1;
    for (size_t value : shape) {
        if (value != 0 && product > std::numeric_limits<size_t>::max() / value) {
            throw std::overflow_error(std::string(context) + ": shape product overflow");
        }
        product *= value;
    }
    return product;
}

size_t checkedByteCount(const std::vector<size_t>& shape, size_t itemsize, const char* context) {
    const size_t count = checkedProduct(shape, context);
    if (count != 0 && itemsize > std::numeric_limits<size_t>::max() / count) {
        throw std::overflow_error(std::string(context) + ": byte-size overflow");
    }
    return count * itemsize;
}

std::shared_ptr<void> allocateBytes(size_t byteCount, bool zeroInitialize = false) {
    if (byteCount == 0) {
        return nullptr;
    }

    std::uint8_t* buffer = zeroInitialize ? new std::uint8_t[byteCount]() : new std::uint8_t[byteCount];
    return std::shared_ptr<void>(buffer, [](void* ptr) {
        delete[] static_cast<std::uint8_t*>(ptr);
    });
}

std::vector<size_t> makeSignedCompatible(const std::vector<size_t>& values, const char* context) {
    std::vector<size_t> output = values;
    for (size_t value : output) {
        if (value > static_cast<size_t>(std::numeric_limits<ssize_t>::max())) {
            throw std::overflow_error(std::string(context) + ": value exceeds ssize_t range");
        }
    }
    return output;
}

std::vector<size_t> computeByteStrides(
    const std::vector<size_t>& shape,
    size_t itemsize,
    char order,
    const char* context) {

    const char normalizedOrder = normalizeOrder(order, context);
    std::vector<size_t> strides(shape.size(), 0);
    size_t stride = itemsize;

    if (normalizedOrder == 'F') {
        for (size_t i = 0; i < shape.size(); ++i) {
            strides[i] = stride;
            if (shape[i] != 0 && stride > std::numeric_limits<size_t>::max() / shape[i]) {
                throw std::overflow_error(std::string(context) + ": overflow while computing Fortran strides");
            }
            stride *= shape[i];
        }
        return strides;
    }

    for (size_t j = 0; j < shape.size(); ++j) {
        const size_t dim = shape.size() - 1 - j;
        strides[dim] = stride;
        if (shape[dim] != 0 && stride > std::numeric_limits<size_t>::max() / shape[dim]) {
            throw std::overflow_error(std::string(context) + ": overflow while computing C strides");
        }
        stride *= shape[dim];
    }
    return strides;
}

std::vector<size_t> indicesFromFlatIndex(
    size_t flatIndex,
    const std::vector<size_t>& shape,
    char order,
    const char* context) {

    const char normalizedOrder = normalizeOrder(order, context);
    std::vector<size_t> indices(shape.size(), 0);

    if (shape.empty()) {
        return indices;
    }

    if (normalizedOrder == 'C') {
        for (size_t j = 0; j < shape.size(); ++j) {
            const size_t dim = shape.size() - 1 - j;
            const size_t axisExtent = shape[dim];
            indices[dim] = axisExtent == 0 ? 0 : flatIndex % axisExtent;
            flatIndex = axisExtent == 0 ? 0 : flatIndex / axisExtent;
        }
        return indices;
    }

    for (size_t dim = 0; dim < shape.size(); ++dim) {
        const size_t axisExtent = shape[dim];
        indices[dim] = axisExtent == 0 ? 0 : flatIndex % axisExtent;
        flatIndex = axisExtent == 0 ? 0 : flatIndex / axisExtent;
    }
    return indices;
}

size_t byteOffsetFromIndices(const Array& array, const std::vector<size_t>& indices) {
    const std::vector<size_t> shape = array.shape();
    const std::vector<size_t> strides = array.strides();
    if (indices.size() != shape.size()) {
        throw std::runtime_error("Array: wrong number of indices");
    }

    size_t offset = 0;
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i] >= shape[i]) {
            throw std::runtime_error("Array: index out of bounds");
        }
        offset += indices[i] * strides[i];
    }
    return offset;
}

std::u32string u32StringFromRaw(const char32_t* data, size_t codePointCapacity) {
    std::u32string output;
    output.reserve(codePointCapacity);
    for (size_t i = 0; i < codePointCapacity; ++i) {
        const char32_t value = data[i];
        if (value == U'\0') {
            break;
        }
        output.push_back(value);
    }
    return output;
}

std::string stringFromU32String(const std::u32string& value) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.to_bytes(value);
}

template <typename T>
std::shared_ptr<Data> makeFilledWithFactory(const std::vector<size_t>& shape, double value, char order) {
    return std::make_shared<Array>(arrayfactory::full<T>(shape, static_cast<T>(value), order));
}

std::shared_ptr<Data> fullFromDtype(
    const std::vector<size_t>& shape,
    double value,
    const std::string& dtypeName,
    char order,
    const char* context) {

    const char normalizedOrder = normalizeOrder(order, context);

    if (dtypeName == "bool") {
        return makeFilledWithFactory<bool>(shape, value, normalizedOrder);
    }
    if (dtypeName == "int8") {
        return makeFilledWithFactory<int8_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "int16") {
        return makeFilledWithFactory<int16_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "int32" || dtypeName == "int") {
        return makeFilledWithFactory<int32_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "int64") {
        return makeFilledWithFactory<int64_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "uint8") {
        return makeFilledWithFactory<uint8_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "uint16") {
        return makeFilledWithFactory<uint16_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "uint32") {
        return makeFilledWithFactory<uint32_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "uint64") {
        return makeFilledWithFactory<uint64_t>(shape, value, normalizedOrder);
    }
    if (dtypeName == "float32" || dtypeName == "float") {
        return makeFilledWithFactory<float>(shape, value, normalizedOrder);
    }
    if (dtypeName == "float64" || dtypeName == "double") {
        return makeFilledWithFactory<double>(shape, value, normalizedOrder);
    }

    throw std::invalid_argument(std::string(context) + ": unsupported dtype '" + dtypeName + "'");
}

template <typename T>
Array makeDeepCopyImpl(const Array& source, char order) {
    const std::vector<size_t> strides = computeByteStrides(source.shape(), sizeof(T), order, "Array::copy");
    const size_t byteCount = checkedByteCount(source.shape(), sizeof(T), "Array::copy");
    auto owner = allocateBytes(byteCount, false);
    Array copied(Array::typeIdFor<T>(), sizeof(T), owner.get(), source.shape(), strides, owner);

    if (source.size() == 0) {
        return copied;
    }

    if ((order == 'C' && source.isContiguousInStyleC()) || (order == 'F' && source.isContiguousInStyleFortran())) {
        std::memcpy(copied.rawData(), source.rawData(), byteCount);
        return copied;
    }

    T* dst = copied.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < source.size(); ++i) {
        const std::vector<size_t> indices = indicesFromFlatIndex(i, source.shape(), order, "Array::copy");
        const size_t sourceOffset = byteOffsetFromIndices(source, indices) / sizeof(T);
        dst[i] = source.getPointerOfReadOnlyDataFast<T>()[sourceOffset];
    }
    return copied;
}

Array makeDeepCopyString(const Array& source, char order) {
    const size_t byteCount = checkedByteCount(source.shape(), source.itemsize(), "Array::copy");
    const std::vector<size_t> strides = computeByteStrides(source.shape(), source.itemsize(), order, "Array::copy");
    auto owner = allocateBytes(byteCount, false);

    Array copied(
        source.typeId(),
        source.itemsize(),
        owner.get(),
        source.shape(),
        strides,
        owner);

    if (source.size() == 0) {
        return copied;
    }

    if ((order == 'C' && source.isContiguousInStyleC()) || (order == 'F' && source.isContiguousInStyleFortran())) {
        std::memcpy(copied.rawData(), source.rawData(), byteCount);
        return copied;
    }

    auto* destination = static_cast<std::uint8_t*>(copied.rawData());
    const auto* sourceBytes = static_cast<const std::uint8_t*>(source.rawData());
    for (size_t i = 0; i < source.size(); ++i) {
        const std::vector<size_t> indices = indicesFromFlatIndex(i, source.shape(), order, "Array::copy");
        const size_t sourceOffset = byteOffsetFromIndices(source, indices);
        std::memcpy(destination + i * source.itemsize(), sourceBytes + sourceOffset, source.itemsize());
    }
    return copied;
}

} // namespace

Array::Array() {
    this->setArrayMembersAsNull();
}

Array::Array(ArrayTypeId typeId,
             size_t itemsize,
             void* data,
             const std::vector<size_t>& shape,
             const std::vector<size_t>& strides,
             std::shared_ptr<void> owner) :
    Array(typeId, itemsize, data, shape, strides, std::move(owner), ArrayOwnerKind::ManagedBuffer) {}

Array::Array(ArrayTypeId typeId,
             size_t itemsize,
             void* data,
             const std::vector<size_t>& shape,
             const std::vector<size_t>& strides,
             std::shared_ptr<void> owner,
             ArrayOwnerKind ownerKind) {
    this->setArrayMembersUsing(typeId, itemsize, data, shape, strides, std::move(owner), ownerKind);
}

Array Array::bytesView(void* data,
                       size_t itemsize,
                       const std::vector<size_t>& shape,
                       const std::vector<size_t>& strides,
                       std::shared_ptr<void> owner) {
    return Array::bytesView(data, itemsize, shape, strides, std::move(owner), ArrayOwnerKind::ManagedBuffer);
}

Array Array::bytesView(void* data,
                       size_t itemsize,
                       const std::vector<size_t>& shape,
                       const std::vector<size_t>& strides,
                       std::shared_ptr<void> owner,
                       ArrayOwnerKind ownerKind) {
    return Array(ArrayTypeId::Bytes, itemsize, data, shape, strides, std::move(owner), ownerKind);
}

Array Array::unicodeView(void* data,
                         size_t itemsize,
                         const std::vector<size_t>& shape,
                         const std::vector<size_t>& strides,
                         std::shared_ptr<void> owner) {
    return Array::unicodeView(data, itemsize, shape, strides, std::move(owner), ArrayOwnerKind::ManagedBuffer);
}

Array Array::unicodeView(void* data,
                         size_t itemsize,
                         const std::vector<size_t>& shape,
                         const std::vector<size_t>& strides,
                         std::shared_ptr<void> owner,
                         ArrayOwnerKind ownerKind) {
    return Array(ArrayTypeId::Unicode, itemsize, data, shape, strides, std::move(owner), ownerKind);
}

size_t Array::computeSizeFromShape(const std::vector<size_t>& shape) {
    return checkedProduct(shape, "Array::computeSizeFromShape");
}

void Array::setArrayMembersAsNull() {
    this->_owner.reset();
    this->_ownerKind = ArrayOwnerKind::None;
    this->_data = nullptr;
    this->_dtype = {};
    this->_dimensions = 0;
    this->_size = 0;
    this->_shape.clear();
    this->_strides.clear();
    this->_must = nullptr;
}

void Array::setArrayMembersUsing(ArrayTypeId typeId,
                                 size_t itemsize,
                                 void* data,
                                 const std::vector<size_t>& shape,
                                 const std::vector<size_t>& strides,
                                 std::shared_ptr<void> owner,
                                 ArrayOwnerKind ownerKind) {
    if (shape.size() != strides.size()) {
        throw std::invalid_argument("Array: shape and strides must have the same rank");
    }
    makeSignedCompatible(shape, "Array");
    makeSignedCompatible(strides, "Array");

    this->_owner = std::move(owner);
    this->_ownerKind = this->_owner ? ownerKind : ArrayOwnerKind::None;
    this->_data = static_cast<std::uint8_t*>(data);
    this->_dtype = {typeId, itemsize};
    this->_dimensions = shape.size();
    this->_size = computeSizeFromShape(shape);
    this->_shape = shape;
    this->_strides = strides;
    this->_must = nullptr;

    if (typeId == ArrayTypeId::None) {
        this->_data = nullptr;
        this->_ownerKind = ArrayOwnerKind::None;
        this->_shape.clear();
        this->_strides.clear();
        this->_dimensions = 0;
        this->_size = 0;
        this->_dtype.itemsize = 0;
        return;
    }

    if (itemsize == 0 && typeId != ArrayTypeId::Bytes && typeId != ArrayTypeId::Unicode) {
        throw std::invalid_argument("Array: itemsize must be positive for non-empty dtypes");
    }

    if (this->_size > 0 && this->_data == nullptr) {
        throw std::invalid_argument("Array: data pointer must not be null when size > 0");
    }
}

Array::Array(const std::string& str) {
    const size_t byteCount = str.size();
    auto owner = allocateBytes(byteCount, false);
    if (byteCount != 0) {
        std::memcpy(owner.get(), str.data(), byteCount);
    }
    this->setArrayMembersUsing(
        ArrayTypeId::Bytes,
        byteCount,
        owner.get(),
        {1},
        {byteCount},
        owner,
        ArrayOwnerKind::ManagedBuffer);
}

Array::Array(const char* str) : Array(std::string(str == nullptr ? "" : str)) {}

Array::Array(const Array& other) = default;

template <typename T>
Array Array::makeScalar(T scalar) {
    auto owner = allocateBytes(sizeof(T), false);
    *static_cast<T*>(owner.get()) = scalar;
    return Array(typeIdFor<T>(), sizeof(T), owner.get(), {1}, {sizeof(T)}, owner, ArrayOwnerKind::ManagedBuffer);
}

Array::Array(int8_t scalar) : Array(makeScalar<int8_t>(scalar)) {}
Array::Array(int16_t scalar) : Array(makeScalar<int16_t>(scalar)) {}
Array::Array(int32_t scalar) : Array(makeScalar<int32_t>(scalar)) {}
Array::Array(int64_t scalar) : Array(makeScalar<int64_t>(scalar)) {}
Array::Array(uint8_t scalar) : Array(makeScalar<uint8_t>(scalar)) {}
Array::Array(uint16_t scalar) : Array(makeScalar<uint16_t>(scalar)) {}
Array::Array(uint32_t scalar) : Array(makeScalar<uint32_t>(scalar)) {}
Array::Array(uint64_t scalar) : Array(makeScalar<uint64_t>(scalar)) {}
Array::Array(float scalar) : Array(makeScalar<float>(scalar)) {}
Array::Array(double scalar) : Array(makeScalar<double>(scalar)) {}
Array::Array(bool scalar) : Array(makeScalar<bool>(scalar)) {}

Array::Assertions& Array::must() const {
    if (!this->_must) {
        this->_must = std::make_shared<Assertions>(*this);
    }
    return *this->_must;
}

std::shared_ptr<Data> Array::clone() const {
    return std::make_shared<Array>(*this);
}

std::shared_ptr<Data> Array::copy(bool deep) const {
    if (!deep) {
        return this->clone();
    }

    if (this->isNone()) {
        return std::make_shared<Array>();
    }

    const char order = this->isContiguousInStyleFortran() && !this->isContiguousInStyleC() ? 'F' : 'C';

    if (this->hasString()) {
        return std::make_shared<Array>(makeDeepCopyString(*this, order));
    }

    if (this->hasDataOfType<bool>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<bool>(*this, order));
    }
    if (this->hasDataOfType<int8_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<int8_t>(*this, order));
    }
    if (this->hasDataOfType<int16_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<int16_t>(*this, order));
    }
    if (this->hasDataOfType<int32_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<int32_t>(*this, order));
    }
    if (this->hasDataOfType<int64_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<int64_t>(*this, order));
    }
    if (this->hasDataOfType<uint8_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<uint8_t>(*this, order));
    }
    if (this->hasDataOfType<uint16_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<uint16_t>(*this, order));
    }
    if (this->hasDataOfType<uint32_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<uint32_t>(*this, order));
    }
    if (this->hasDataOfType<uint64_t>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<uint64_t>(*this, order));
    }
    if (this->hasDataOfType<float>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<float>(*this, order));
    }
    if (this->hasDataOfType<double>()) {
        return std::make_shared<Array>(makeDeepCopyImpl<double>(*this, order));
    }

    throw std::runtime_error("Array::copy unsupported dtype");
}

bool Array::hasString() const {
    return this->_dtype.isString();
}

bool Array::isNone() const {
    return this->_dtype.id == ArrayTypeId::None || this->_size < 1;
}

bool Array::isScalar() const {
    if (this->isNone()) {
        return false;
    }
    if (this->hasString()) {
        return false;
    }
    return this->size() == 1;
}

std::string Array::dtype() const {
    return this->_dtype.name();
}

std::shared_ptr<Data> Array::full(
    const std::vector<size_t>& shape,
    double value,
    const std::string& dtypeName,
    char order) const {
    return fullFromDtype(shape, value, dtypeName, order, "Array::full");
}

size_t Array::getByteOffsetFromIndices(const std::vector<size_t>& indices) const {
    if (indices.size() != this->_dimensions) {
        throw std::runtime_error("Array: wrong number of indices");
    }

    size_t offset = 0;
    for (size_t i = 0; i < this->_dimensions; ++i) {
        if (indices[i] >= this->_shape[i]) {
            throw std::runtime_error("Array: index out of bounds");
        }
        if (this->_strides[i] != 0 && indices[i] > std::numeric_limits<size_t>::max() / this->_strides[i]) {
            throw std::overflow_error("Array: offset overflow");
        }
        offset += indices[i] * this->_strides[i];
    }
    return offset;
}

size_t Array::getByteOffsetFromFlatIndex(size_t flatIndex) const {
    if (this->_dimensions == 0) {
        return 0;
    }

    size_t offset = 0;
    for (size_t j = 0; j < this->_dimensions; ++j) {
        const size_t dim = this->_dimensions - 1 - j;
        const size_t axisExtent = this->_shape[dim];
        const size_t index = axisExtent == 0 ? 0 : flatIndex % axisExtent;
        offset += index * this->_strides[dim];
        flatIndex = axisExtent == 0 ? 0 : flatIndex / axisExtent;
    }
    return offset;
}

std::shared_ptr<Data> Array::ravel(const std::string& order) const {
    if (this->isNone()) {
        return this->clone();
    }

    if (order.empty()) {
        throw std::invalid_argument("Array::ravel: order must not be empty");
    }

    char normalizedOrder = static_cast<char>(std::toupper(static_cast<unsigned char>(order[0])));
    if (normalizedOrder == 'A') {
        normalizedOrder = this->isContiguousInStyleFortran() && !this->isContiguousInStyleC() ? 'F' : 'C';
    }
    if (normalizedOrder == 'K') {
        if (this->isContiguous()) {
        return std::make_shared<Array>(
                Array(this->_dtype.id, this->_dtype.itemsize, this->_data, {this->_size}, {this->_dtype.itemsize}, this->_owner, this->_ownerKind));
        }
        normalizedOrder = 'C';
    }
    normalizeOrder(normalizedOrder, "Array::ravel");

    if ((normalizedOrder == 'C' && this->isContiguousInStyleC()) ||
        (normalizedOrder == 'F' && this->isContiguousInStyleFortran())) {
        return std::make_shared<Array>(
            Array(this->_dtype.id, this->_dtype.itemsize, this->_data, {this->_size}, {this->_dtype.itemsize}, this->_owner, this->_ownerKind));
    }

    const size_t byteCount = this->_size * this->_dtype.itemsize;
    auto owner = allocateBytes(byteCount, false);
    Array flattened(this->_dtype.id, this->_dtype.itemsize, owner.get(), {this->_size}, {this->_dtype.itemsize}, owner);

    auto* destination = static_cast<std::uint8_t*>(flattened.rawData());
    const auto* source = static_cast<const std::uint8_t*>(this->rawData());
    for (size_t i = 0; i < this->_size; ++i) {
        const auto indices = indicesFromFlatIndex(i, this->_shape, normalizedOrder, "Array::ravel");
        const size_t sourceOffset = this->getByteOffsetFromIndices(indices);
        std::memcpy(destination + i * this->_dtype.itemsize, source + sourceOffset, this->_dtype.itemsize);
    }

    return std::make_shared<Array>(flattened);
}

std::shared_ptr<Data> Array::take(int64_t index, size_t axis) const {
    if (this->dimensions() == 0) {
        return this->clone();
    }
    if (axis >= this->dimensions()) {
        throw std::runtime_error("Array::take: axis out of bounds");
    }

    const auto currentShape = this->shape();
    const size_t axisLength = currentShape[axis];
    if (axisLength == 0) {
        throw std::runtime_error("Array::take: cannot take from empty axis");
    }
    if (axisLength > static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
        throw std::runtime_error("Array::take: axis length exceeds int64 range");
    }

    int64_t normalizedIndex = index;
    if (normalizedIndex < 0) {
        normalizedIndex += static_cast<int64_t>(axisLength);
    }
    if (normalizedIndex < 0 || normalizedIndex >= static_cast<int64_t>(axisLength)) {
        throw std::runtime_error("Array::take: index out of bounds");
    }

    std::vector<size_t> newShape = this->_shape;
    std::vector<size_t> newStrides = this->_strides;
    std::uint8_t* newData = this->_data + static_cast<size_t>(normalizedIndex) * this->_strides[axis];

    if (this->dimensions() == 1) {
        newShape = {1};
        newStrides = {this->_dtype.itemsize};
    } else {
        newShape.erase(newShape.begin() + static_cast<ptrdiff_t>(axis));
        newStrides.erase(newStrides.begin() + static_cast<ptrdiff_t>(axis));
    }

    return std::make_shared<Array>(
        Array(this->_dtype.id, this->_dtype.itemsize, newData, newShape, newStrides, this->_owner, this->_ownerKind));
}

int64_t Array::itemAsInt64(const std::vector<size_t>& indices) const {
    const size_t byteOffset = this->dimensions() == 0 ? 0 : this->getByteOffsetFromIndices(indices);
    const auto* bytes = static_cast<const std::uint8_t*>(this->rawData()) + byteOffset;

    if (this->hasDataOfType<bool>()) return static_cast<int64_t>(*reinterpret_cast<const bool*>(bytes));
    if (this->hasDataOfType<int8_t>()) return static_cast<int64_t>(*reinterpret_cast<const int8_t*>(bytes));
    if (this->hasDataOfType<int16_t>()) return static_cast<int64_t>(*reinterpret_cast<const int16_t*>(bytes));
    if (this->hasDataOfType<int32_t>()) return static_cast<int64_t>(*reinterpret_cast<const int32_t*>(bytes));
    if (this->hasDataOfType<int64_t>()) return *reinterpret_cast<const int64_t*>(bytes);
    if (this->hasDataOfType<uint8_t>()) return static_cast<int64_t>(*reinterpret_cast<const uint8_t*>(bytes));
    if (this->hasDataOfType<uint16_t>()) return static_cast<int64_t>(*reinterpret_cast<const uint16_t*>(bytes));
    if (this->hasDataOfType<uint32_t>()) return static_cast<int64_t>(*reinterpret_cast<const uint32_t*>(bytes));
    if (this->hasDataOfType<uint64_t>()) return static_cast<int64_t>(*reinterpret_cast<const uint64_t*>(bytes));
    if (this->hasDataOfType<float>()) return static_cast<int64_t>(*reinterpret_cast<const float*>(bytes));
    if (this->hasDataOfType<double>()) return static_cast<int64_t>(*reinterpret_cast<const double*>(bytes));

    throw std::runtime_error("Array::itemAsInt64: unsupported dtype");
}

void Array::setItemFromInt64(const std::vector<size_t>& indices, int64_t value) {
    const size_t byteOffset = this->dimensions() == 0 ? 0 : this->getByteOffsetFromIndices(indices);
    auto* bytes = static_cast<std::uint8_t*>(this->rawData()) + byteOffset;

    if (this->hasDataOfType<bool>()) {
        *reinterpret_cast<bool*>(bytes) = static_cast<bool>(value);
        return;
    }
    if (this->hasDataOfType<int8_t>()) {
        *reinterpret_cast<int8_t*>(bytes) = static_cast<int8_t>(value);
        return;
    }
    if (this->hasDataOfType<int16_t>()) {
        *reinterpret_cast<int16_t*>(bytes) = static_cast<int16_t>(value);
        return;
    }
    if (this->hasDataOfType<int32_t>()) {
        *reinterpret_cast<int32_t*>(bytes) = static_cast<int32_t>(value);
        return;
    }
    if (this->hasDataOfType<int64_t>()) {
        *reinterpret_cast<int64_t*>(bytes) = value;
        return;
    }
    if (this->hasDataOfType<uint8_t>()) {
        *reinterpret_cast<uint8_t*>(bytes) = static_cast<uint8_t>(value);
        return;
    }
    if (this->hasDataOfType<uint16_t>()) {
        *reinterpret_cast<uint16_t*>(bytes) = static_cast<uint16_t>(value);
        return;
    }
    if (this->hasDataOfType<uint32_t>()) {
        *reinterpret_cast<uint32_t*>(bytes) = static_cast<uint32_t>(value);
        return;
    }
    if (this->hasDataOfType<uint64_t>()) {
        *reinterpret_cast<uint64_t*>(bytes) = static_cast<uint64_t>(value);
        return;
    }
    if (this->hasDataOfType<float>()) {
        *reinterpret_cast<float*>(bytes) = static_cast<float>(value);
        return;
    }
    if (this->hasDataOfType<double>()) {
        *reinterpret_cast<double*>(bytes) = static_cast<double>(value);
        return;
    }

    throw std::runtime_error("Array::setItemFromInt64: unsupported dtype");
}

bool Array::isContiguous() const {
    return this->isContiguousInStyleC() || this->isContiguousInStyleFortran();
}

bool Array::isContiguousInStyleC() const {
    std::vector<size_t> strides = this->strides();
    std::vector<size_t> shape = this->shape();
    size_t expectedStride = this->_dtype.itemsize;
    size_t dims = this->dimensions();

    if (dims == 0) {
        return true;
    }

    for (size_t j = 0; j < dims; ++j) {
        size_t dim = dims - 1 - j;
        if (strides[dim] != expectedStride) {
            return false;
        }
        const size_t currentShape = shape[dim];
        if (currentShape > 0 && expectedStride > (SIZE_MAX / currentShape)) {
            throw std::overflow_error("expectedStride overflow in isContiguousInStyleC");
        }
        expectedStride *= currentShape;
    }
    return true;
}

bool Array::isContiguousInStyleFortran() const {
    std::vector<size_t> strides = this->strides();
    std::vector<size_t> shape = this->shape();
    size_t expectedStride = this->_dtype.itemsize;

    for (size_t i = 0; i < this->dimensions(); ++i) {
        if (strides[i] != expectedStride) {
            return false;
        }
        expectedStride *= shape[i];
    }
    return true;
}

std::string Array::info() const {
    std::string txt;
    if (this->hasString()) {
        txt += "Array dtype: " + this->dtype() + "\n";
        txt += this->extractString();
        return txt;
    }

    txt += "Array dtype: " + this->dtype() + "\n";
    txt += this->_owner ? "owns or shares managed data\n" : "data references unmanaged memory\n";
    txt += this->isContiguousInStyleC() ? "C contiguous\n" : "C non-contiguous\n";
    txt += this->isContiguousInStyleFortran() ? "Fortran contiguous\n" : "Fortran non-contiguous\n";
    txt += "shape: ";

    size_t total = 1;
    for (size_t dim = 0; dim < this->_shape.size(); ++dim) {
        if (dim > 0) {
            txt += "x";
        }
        total *= this->_shape[dim];
        txt += std::to_string(this->_shape[dim]);
    }
    txt += " =" + std::to_string(total) + "\n";
    txt += this->getPrintString();
    return txt;
}

std::string Array::shortInfo() const {
    std::string txt = "Array " + this->dtype();
    if (this->size() < 7) {
        txt += " " + this->getPrintString(30);
    }
    return txt;
}
