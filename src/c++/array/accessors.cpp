#include "array/array.hpp"

#include "array/factory/strings.hpp"

#include <codecvt>
#include <locale>
#include <stdexcept>

Array Array::operator[](const size_t flatIndex) {
    if (hasDataOfType<int8_t>()) { return this->getItemAsArrayAtIndex<int8_t>(flatIndex); }
    if (hasDataOfType<int16_t>()) { return this->getItemAsArrayAtIndex<int16_t>(flatIndex); }
    if (hasDataOfType<int32_t>()) { return this->getItemAsArrayAtIndex<int32_t>(flatIndex); }
    if (hasDataOfType<int64_t>()) { return this->getItemAsArrayAtIndex<int64_t>(flatIndex); }
    if (hasDataOfType<uint8_t>()) { return this->getItemAsArrayAtIndex<uint8_t>(flatIndex); }
    if (hasDataOfType<uint16_t>()) { return this->getItemAsArrayAtIndex<uint16_t>(flatIndex); }
    if (hasDataOfType<uint32_t>()) { return this->getItemAsArrayAtIndex<uint32_t>(flatIndex); }
    if (hasDataOfType<uint64_t>()) { return this->getItemAsArrayAtIndex<uint64_t>(flatIndex); }
    if (hasDataOfType<float>()) { return this->getItemAsArrayAtIndex<float>(flatIndex); }
    if (hasDataOfType<double>()) { return this->getItemAsArrayAtIndex<double>(flatIndex); }
    if (hasDataOfType<bool>()) { return this->getItemAsArrayAtIndex<bool>(flatIndex); }
    throw std::runtime_error("Array::operator[] unsupported array data type");
}

template <typename T>
Array Array::getItemAsArrayAtIndex(const size_t& flatIndex) {
    T* data = this->getPointerOfDataSafely<T>();
    const size_t offset = this->isContiguous() ? flatIndex : this->getOffsetFromFlatIndex<T>(flatIndex);
    return Array(typeIdFor<T>(), sizeof(T), data + offset, {1}, {sizeof(T)}, this->_owner, this->_ownerKind);
}

template <typename T>
T& Array::getItemAtIndex(const size_t& index) {
    T* data = this->getPointerOfDataSafely<T>();
    if (this->isContiguous()) {
        return data[index];
    }
    const size_t offset = getOffsetFromFlatIndex<T>(index);
    return data[offset];
}

template <typename T>
size_t Array::getOffsetFromFlatIndex(size_t flatIndex) const {
    return this->getByteOffsetFromFlatIndex(flatIndex) / sizeof(T);
}

template <typename T>
const T& Array::getItemAtIndex(const size_t& index) const {
    const T* data = this->getPointerOfReadOnlyDataFast<T>();
    if (this->isContiguous()) {
        return data[index];
    }
    const size_t offset = getOffsetFromFlatIndex<T>(index);
    return data[offset];
}

template <typename T>
T* Array::getPointerOfDataSafely() {
    must().haveDataOfType<T>();
    return static_cast<T*>(this->rawData());
}

template <typename T>
T* Array::getPointerOfModifiableDataFast() {
    return reinterpret_cast<T*>(this->rawData());
}

template <typename T>
const T* Array::getPointerOfReadOnlyDataFast() const {
    return reinterpret_cast<const T*>(this->rawData());
}

size_t Array::getFlatIndex(const std::vector<size_t>& indices) const {
    if (this->isContiguousInStyleC()) {
        return this->getFlatIndexOfArrayInStyleC(indices);
    }
    if (this->isContiguousInStyleFortran()) {
        return this->getFlatIndexOfArrayInStyleFortran(indices);
    }
    throw std::invalid_argument("getFlatIndex to be implemented for non-contiguous arrays");
}

size_t Array::getFlatIndexOfArrayInStyleC(const std::vector<size_t>& indices) const {
    size_t flatIndex = 0;
    size_t multiplier = 1;

    if (this->_dimensions == 0) {
        return 0;
    }

    const size_t dims = this->_dimensions;
    for (size_t j = 0; j < dims; ++j) {
        const size_t dim = dims - 1 - j;
        flatIndex += indices[dim] * multiplier;
        const size_t currentShape = this->_shape[dim];
        if (currentShape > 0 && multiplier > (SIZE_MAX / currentShape)) {
            throw std::overflow_error("Multiplication overflow in getFlatIndexOfArrayInStyleC");
        }
        multiplier *= currentShape;
    }

    return flatIndex;
}

size_t Array::getFlatIndexOfArrayInStyleFortran(const std::vector<size_t>& indices) const {
    size_t flatIndex = 0;
    size_t multiplier = 1;

    for (size_t i = 0; i < this->_dimensions; ++i) {
        flatIndex += indices[i] * multiplier;
        multiplier *= this->_shape[i];
    }

    return flatIndex;
}

std::string Array::extractString() const {
    const char kind = this->dtypeKind();
    if (kind == 'U') {
        return this->extractStringOfKindU();
    }
    if (kind == 'S') {
        return this->extractStringOfKindS();
    }
    return std::string("");
}

std::string Array::extractStringOfKindU() const {
    if (this->dtypeKind() != 'U') {
        throw std::invalid_argument("input array string kind is not 'U'");
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::string outputString;

    for (size_t i = 0; i < this->size(); ++i) {
        const std::string separator = i == 0 ? std::string("") : std::string(" ");
        const size_t offset = this->getByteOffsetFromFlatIndex(i);
        const auto* dataPtr = reinterpret_cast<const char32_t*>(
            static_cast<const std::uint8_t*>(this->rawData()) + offset);
        const size_t codePointCapacity = this->itemsize() / sizeof(char32_t);
        std::u32string value;
        value.reserve(codePointCapacity);
        for (size_t j = 0; j < codePointCapacity; ++j) {
            const char32_t codePoint = dataPtr[j];
            if (codePoint == U'\0') {
                break;
            }
            value.push_back(codePoint);
        }
        outputString += separator + converter.to_bytes(value);
    }

    return outputString;
}

std::string Array::extractStringOfKindS() const {
    if (this->dtypeKind() != 'S') {
        throw std::invalid_argument("input array string kind is not 'S'");
    }

    std::string outputString;
    for (size_t i = 0; i < this->size(); ++i) {
        const std::string separator = i == 0 ? std::string("") : std::string(" ");
        const size_t offset = this->getByteOffsetFromFlatIndex(i);
        const auto* dataPtr = static_cast<const char*>(
            static_cast<const void*>(static_cast<const std::uint8_t*>(this->rawData()) + offset));
        std::string value(dataPtr, this->itemsize());
        while (!value.empty() && value.back() == '\0') {
            value.pop_back();
        }
        outputString += separator + value;
    }
    return outputString;
}

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&Array::template getPointerOfReadOnlyDataFast<U>), ...);
        (utils::forceSymbol(&Array::template getPointerOfModifiableDataFast<U>), ...);
        (utils::forceSymbol(&Array::template getPointerOfDataSafely<U>), ...);
        (forceGetItemSymbols<U>(), ...);
    }

    template <typename U>
    static void forceGetItemSymbols() {
        utils::forceSymbol(static_cast<U& (Array::*)(const size_t&)>(&Array::template getItemAtIndex<U>));
        utils::forceSymbol(static_cast<const U& (Array::*)(const size_t&) const>(&Array::template getItemAtIndex<U>));
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

template size_t Array::getOffsetFromFlatIndex<bool>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<float>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<double>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<int8_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<int16_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<int32_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<int64_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<uint8_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<uint16_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<uint32_t>(size_t) const;
template size_t Array::getOffsetFromFlatIndex<uint64_t>(size_t) const;
