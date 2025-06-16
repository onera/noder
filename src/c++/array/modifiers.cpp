# include "array/array.hpp"



template <>
Array& Array::operator=(const int8_t& scalar) { return this->setElementsAs<int8_t>(scalar);}
template <>
Array& Array::operator=(const int16_t& scalar) { return this->setElementsAs<int16_t>(scalar);}
template <>
Array& Array::operator=(const int32_t& scalar) { return this->setElementsAs<int32_t>(scalar);}
template <>
Array& Array::operator=(const int64_t& scalar) { return this->setElementsAs<int64_t>(scalar);}
template <>
Array& Array::operator=(const uint8_t& scalar) { return this->setElementsAs<uint8_t>(scalar);}
template <>
Array& Array::operator=(const uint16_t& scalar) { return this->setElementsAs<uint16_t>(scalar);}
template <>
Array& Array::operator=(const uint32_t& scalar) { return this->setElementsAs<uint32_t>(scalar);}
template <>
Array& Array::operator=(const uint64_t& scalar) { return this->setElementsAs<uint64_t>(scalar);}
template <>
Array& Array::operator=(const float& scalar) { return this->setElementsAs<float>(scalar);}
template <>
Array& Array::operator=(const double& scalar) { return this->setElementsAs<double>(scalar);}
template <>
Array& Array::operator=(const bool& scalar) { return this->setElementsAs<bool>(scalar);}

template <>
Array& Array::operator=(const Array& other) {
    if      (hasDataOfType<int8_t>())   { return this->setElementsFrom<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->setElementsFrom<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->setElementsFrom<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->setElementsFrom<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->setElementsFrom<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->setElementsFrom<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->setElementsFrom<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->setElementsFrom<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->setElementsFrom<float>(other);}
    else if (hasDataOfType<double>())   { return this->setElementsFrom<double>(other);}
    else if (hasDataOfType<bool>())     { return this->setElementsFrom<bool>(other);}
    else { throw std::runtime_error("Array:: operator= unsupported array data type"); }
}


template <typename T>
Array& Array::setElementsAs(const T& scalar) {
    this->must().haveValidDataTypeForSettingScalar<T>();
    T* data = this->getPointerOfModifiableDataFast<T>();
    
    if (this->isContiguous()) {
        for (size_t i = 0; i < this->_size; i++) {
            data[i] = scalar;
        }
    } else {
        size_t offset;
        for (size_t i = 0; i < this->_size; i++) {
            offset = this->getOffsetFromFlatIndex<T>(i);
            data[offset] = scalar;
        }
    }
    return *this;
}


template <typename T>
Array& Array::setElementsFrom(const Array& other) {
   this->must().haveSameSizeAs<Array>(other);
   other.must().haveDataOfType<T>();
        
    if (this->isContiguous()) {
        if (other.isContiguous()) {
            return this->setElementsOfThisContiguousUsingContiguous<T>(other);
        } else {
            return this->setElementsOfThisContiguousUsingNonContiguous<T>(other);
        }
    
    } else {
        if (other.isContiguous()) {
            return this->setElementsOfThisNonContiguousUsingContiguous<T>(other);
        } else {
            return this->setElementsOfThisNonContiguousUsingNonContiguous<T>(other);
        }
    }

    throw std::runtime_error("theoretically unreachable, bad conditioning");
}

template <typename T>
Array& Array::setElementsOfThisContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    for (size_t i = 0; i < this->_size; i++) {
        thisData[i] = otherData[i];
    }
    
    return *this;
}

template <typename T>
Array& Array::setElementsOfThisContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = other.getOffsetFromFlatIndex<T>(i);
        thisData[i] = otherData[offset];
    }

    return *this;
}

template <typename T>
Array& Array::setElementsOfThisNonContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = this->getOffsetFromFlatIndex<T>(i);
        thisData[offset] = otherData[i];
    }

    return *this;
}

template <typename T>
Array& Array::setElementsOfThisNonContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t thisOffset;
    size_t otherOffset;
    for (size_t i = 0; i < this->_size; i++) {
        thisOffset = this->getOffsetFromFlatIndex<T>(i);
        otherOffset = other.getOffsetFromFlatIndex<T>(i);
        thisData[thisOffset] = otherData[otherOffset];
    }

    return *this;
}


template <>
Array& Array::operator+=(const int8_t& scalar) { return this->increaseElementsBy<int8_t>(scalar);}
template <>
Array& Array::operator+=(const int16_t& scalar) { return this->increaseElementsBy<int16_t>(scalar);}
template <>
Array& Array::operator+=(const int32_t& scalar) { return this->increaseElementsBy<int32_t>(scalar);}
template <>
Array& Array::operator+=(const int64_t& scalar) { return this->increaseElementsBy<int64_t>(scalar);}
template <>
Array& Array::operator+=(const uint8_t& scalar) { return this->increaseElementsBy<uint8_t>(scalar);}
template <>
Array& Array::operator+=(const uint16_t& scalar) { return this->increaseElementsBy<uint16_t>(scalar);}
template <>
Array& Array::operator+=(const uint32_t& scalar) { return this->increaseElementsBy<uint32_t>(scalar);}
template <>
Array& Array::operator+=(const uint64_t& scalar) { return this->increaseElementsBy<uint64_t>(scalar);}
template <>
Array& Array::operator+=(const float& scalar) { return this->increaseElementsBy<float>(scalar);}
template <>
Array& Array::operator+=(const double& scalar) { return this->increaseElementsBy<double>(scalar);}

template <typename T>
Array& Array::increaseElementsBy(const T& scalar) {
    this->must().haveValidDataTypeForSettingScalar<T>();
    T* data = this->getPointerOfModifiableDataFast<T>();
    
    if (this->isContiguous()) {
        for (size_t i = 0; i < this->_size; i++) {
            data[i] += scalar;
        }
    } else {
        size_t offset;
        for (size_t i = 0; i < this->_size; i++) {
            offset = this->getOffsetFromFlatIndex<T>(i);
            data[offset] += scalar;
        }
    }
    return *this;
}


template <>
Array& Array::operator+=(const Array& other) {
    if      (hasDataOfType<int8_t>())   { return this->increaseElementsFrom<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->increaseElementsFrom<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->increaseElementsFrom<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->increaseElementsFrom<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->increaseElementsFrom<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->increaseElementsFrom<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->increaseElementsFrom<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->increaseElementsFrom<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->increaseElementsFrom<float>(other);}
    else if (hasDataOfType<double>())   { return this->increaseElementsFrom<double>(other);}
    else { throw std::runtime_error("Array:: operator+= unsupported array data type"); }
}

template <typename T>
Array& Array::increaseElementsFrom(const Array& other) {
   this->must().haveSameSizeAs<Array>(other);
   other.must().haveDataOfType<T>();
        
    if (this->isContiguous()) {
        if (other.isContiguous()) {
            return this->increaseElementsOfThisContiguousUsingContiguous<T>(other);
        } else {
            return this->increaseElementsOfThisContiguousUsingNonContiguous<T>(other);
        }
    
    } else {
        if (other.isContiguous()) {
            return this->increaseElementsOfThisNonContiguousUsingContiguous<T>(other);
        } else {
            return this->increaseElementsOfThisNonContiguousUsingNonContiguous<T>(other);
        }
    }

    throw std::runtime_error("theoretically unreachable, bad conditioning");
}

template <typename T>
Array& Array::increaseElementsOfThisContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    for (size_t i = 0; i < this->_size; i++) {
        thisData[i] += otherData[i];
    }
    
    return *this;
}

template <typename T>
Array& Array::increaseElementsOfThisContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = other.getOffsetFromFlatIndex<T>(i);
        thisData[i] += otherData[offset];
    }

    return *this;
}

template <typename T>
Array& Array::increaseElementsOfThisNonContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = this->getOffsetFromFlatIndex<T>(i);
        thisData[offset] += otherData[i];
    }

    return *this;
}

template <typename T>
Array& Array::increaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t thisOffset;
    size_t otherOffset;
    for (size_t i = 0; i < this->_size; i++) {
        thisOffset = this->getOffsetFromFlatIndex<T>(i);
        otherOffset = other.getOffsetFromFlatIndex<T>(i);
        thisData[thisOffset] += otherData[otherOffset];
    }

    return *this;
}

template <>
Array& Array::operator-=(const int8_t& scalar) { return this->increaseElementsBy<int8_t>(-scalar);}
template <>
Array& Array::operator-=(const int16_t& scalar) { return this->increaseElementsBy<int16_t>(-scalar);}
template <>
Array& Array::operator-=(const int32_t& scalar) { return this->increaseElementsBy<int32_t>(-scalar);}
template <>
Array& Array::operator-=(const int64_t& scalar) { return this->increaseElementsBy<int64_t>(-scalar);}
template <>
Array& Array::operator-=(const uint8_t& scalar) { return this->increaseElementsBy<uint8_t>(-scalar);}
template <>
Array& Array::operator-=(const uint16_t& scalar) { return this->increaseElementsBy<uint16_t>(-scalar);}
template <>
Array& Array::operator-=(const uint32_t& scalar) { return this->increaseElementsBy<uint32_t>(-scalar);}
template <>
Array& Array::operator-=(const uint64_t& scalar) { return this->increaseElementsBy<uint64_t>(-scalar);}
template <>
Array& Array::operator-=(const float& scalar) { return this->increaseElementsBy<float>(-scalar);}
template <>
Array& Array::operator-=(const double& scalar) { return this->increaseElementsBy<double>(-scalar);}

template <>
Array& Array::operator-=(const Array& other) {
    if      (hasDataOfType<int8_t>())   { return this->decreaseElementsFrom<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->decreaseElementsFrom<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->decreaseElementsFrom<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->decreaseElementsFrom<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->decreaseElementsFrom<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->decreaseElementsFrom<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->decreaseElementsFrom<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->decreaseElementsFrom<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->decreaseElementsFrom<float>(other);}
    else if (hasDataOfType<double>())   { return this->decreaseElementsFrom<double>(other);}
    else { throw std::runtime_error("Array:: operator-= unsupported array data type"); }
}

template <typename T>
Array& Array::decreaseElementsFrom(const Array& other) {
   this->must().haveSameSizeAs<Array>(other);
   other.must().haveDataOfType<T>();
        
    if (this->isContiguous()) {
        if (other.isContiguous()) {
            return this->decreaseElementsOfThisContiguousUsingContiguous<T>(other);
        } else {
            return this->decreaseElementsOfThisContiguousUsingNonContiguous<T>(other);
        }
    
    } else {
        if (other.isContiguous()) {
            return this->decreaseElementsOfThisNonContiguousUsingContiguous<T>(other);
        } else {
            return this->decreaseElementsOfThisNonContiguousUsingNonContiguous<T>(other);
        }
    }

    throw std::runtime_error("theoretically unreachable, bad conditioning");
}

template <typename T>
Array& Array::decreaseElementsOfThisContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    for (size_t i = 0; i < this->_size; i++) {
        thisData[i] -= otherData[i];
    }
    
    return *this;
}

template <typename T>
Array& Array::decreaseElementsOfThisContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = other.getOffsetFromFlatIndex<T>(i);
        thisData[i] -= otherData[offset];
    }

    return *this;
}

template <typename T>
Array& Array::decreaseElementsOfThisNonContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = this->getOffsetFromFlatIndex<T>(i);
        thisData[offset] -= otherData[i];
    }

    return *this;
}

template <typename T>
Array& Array::decreaseElementsOfThisNonContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t thisOffset;
    size_t otherOffset;
    for (size_t i = 0; i < this->_size; i++) {
        thisOffset = this->getOffsetFromFlatIndex<T>(i);
        otherOffset = other.getOffsetFromFlatIndex<T>(i);
        thisData[thisOffset] -= otherData[otherOffset];
    }

    return *this;
}


template <>
Array& Array::operator*=(const int8_t& scalar) { return this->multiplyElementsBy<int8_t>(scalar);}
template <>
Array& Array::operator*=(const int16_t& scalar) { return this->multiplyElementsBy<int16_t>(scalar);}
template <>
Array& Array::operator*=(const int32_t& scalar) { return this->multiplyElementsBy<int32_t>(scalar);}
template <>
Array& Array::operator*=(const int64_t& scalar) { return this->multiplyElementsBy<int64_t>(scalar);}
template <>
Array& Array::operator*=(const uint8_t& scalar) { return this->multiplyElementsBy<uint8_t>(scalar);}
template <>
Array& Array::operator*=(const uint16_t& scalar) { return this->multiplyElementsBy<uint16_t>(scalar);}
template <>
Array& Array::operator*=(const uint32_t& scalar) { return this->multiplyElementsBy<uint32_t>(scalar);}
template <>
Array& Array::operator*=(const uint64_t& scalar) { return this->multiplyElementsBy<uint64_t>(scalar);}
template <>
Array& Array::operator*=(const float& scalar) { return this->multiplyElementsBy<float>(scalar);}
template <>
Array& Array::operator*=(const double& scalar) { return this->multiplyElementsBy<double>(scalar);}

template <>
Array& Array::operator*=(const Array& other) {
    if      (hasDataOfType<int8_t>())   { return this->multiplyElementsFrom<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->multiplyElementsFrom<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->multiplyElementsFrom<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->multiplyElementsFrom<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->multiplyElementsFrom<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->multiplyElementsFrom<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->multiplyElementsFrom<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->multiplyElementsFrom<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->multiplyElementsFrom<float>(other);}
    else if (hasDataOfType<double>())   { return this->multiplyElementsFrom<double>(other);}
    else { throw std::runtime_error("Array:: operator*= unsupported array data type"); }
}

template <typename T>
Array& Array::multiplyElementsFrom(const Array& other) {
   this->must().haveSameSizeAs<Array>(other);
   other.must().haveDataOfType<T>();
        
    if (this->isContiguous()) {
        if (other.isContiguous()) {
            return this->multiplyElementsOfThisContiguousUsingContiguous<T>(other);
        } else {
            return this->multiplyElementsOfThisContiguousUsingNonContiguous<T>(other);
        }
    
    } else {
        if (other.isContiguous()) {
            return this->multiplyElementsOfThisNonContiguousUsingContiguous<T>(other);
        } else {
            return this->multiplyElementsOfThisNonContiguousUsingNonContiguous<T>(other);
        }
    }

    throw std::runtime_error("theoretically unreachable, bad conditioning");
}

template <typename T>
Array& Array::multiplyElementsOfThisContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    for (size_t i = 0; i < this->_size; i++) {
        thisData[i] *= otherData[i];
    }
    
    return *this;
}

template <typename T>
Array& Array::multiplyElementsOfThisContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = other.getOffsetFromFlatIndex<T>(i);
        thisData[i] *= otherData[offset];
    }

    return *this;
}

template <typename T>
Array& Array::multiplyElementsOfThisNonContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = this->getOffsetFromFlatIndex<T>(i);
        thisData[offset] *= otherData[i];
    }

    return *this;
}

template <typename T>
Array& Array::multiplyElementsOfThisNonContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t thisOffset;
    size_t otherOffset;
    for (size_t i = 0; i < this->_size; i++) {
        thisOffset = this->getOffsetFromFlatIndex<T>(i);
        otherOffset = other.getOffsetFromFlatIndex<T>(i);
        thisData[thisOffset] *= otherData[otherOffset];
    }

    return *this;
}


template <typename T>
Array& Array::multiplyElementsBy(const T& scalar) {
    this->must().haveValidDataTypeForSettingScalar<T>();
    T* data = this->getPointerOfModifiableDataFast<T>();
    
    if (this->isContiguous()) {
        for (size_t i = 0; i < this->_size; i++) {
            data[i] *= scalar;
        }
    } else {
        size_t offset;
        for (size_t i = 0; i < this->_size; i++) {
            offset = this->getOffsetFromFlatIndex<T>(i);
            data[offset] *= scalar;
        }
    }
    return *this;
}


template <typename T>
Array& Array::divideElementsBy(const T& scalar) {
    this->must().haveValidDataTypeForSettingScalar<T>();
    T* data = this->getPointerOfModifiableDataFast<T>();
    
    if (this->isContiguous()) {
        for (size_t i = 0; i < this->_size; i++) {
            data[i] /= scalar;
        }
    } else {
        size_t offset;
        for (size_t i = 0; i < this->_size; i++) {
            offset = this->getOffsetFromFlatIndex<T>(i);
            data[offset] /= scalar;
        }
    }
    return *this;
}

template <>
Array& Array::divideElementsBy(const float& scalar) {
    this->must().haveValidDataTypeForSettingScalar<float>();
    return this->multiplyElementsBy<float>(1.0f/scalar);
}

template <>
Array& Array::divideElementsBy(const double& scalar) {
    this->must().haveValidDataTypeForSettingScalar<double>();
    return this->multiplyElementsBy<double>(1.0/scalar);
}


template <>
Array& Array::operator/=(const int8_t& scalar) { return this->divideElementsBy<int8_t>(scalar);}
template <>
Array& Array::operator/=(const int16_t& scalar) { return this->divideElementsBy<int16_t>(scalar);}
template <>
Array& Array::operator/=(const int32_t& scalar) { return this->divideElementsBy<int32_t>(scalar);}
template <>
Array& Array::operator/=(const int64_t& scalar) { return this->divideElementsBy<int64_t>(scalar);}
template <>
Array& Array::operator/=(const uint8_t& scalar) { return this->divideElementsBy<uint8_t>(scalar);}
template <>
Array& Array::operator/=(const uint16_t& scalar) { return this->divideElementsBy<uint16_t>(scalar);}
template <>
Array& Array::operator/=(const uint32_t& scalar) { return this->divideElementsBy<uint32_t>(scalar);}
template <>
Array& Array::operator/=(const uint64_t& scalar) { return this->divideElementsBy<uint64_t>(scalar);}
template <>
Array& Array::operator/=(const float& scalar) { return this->divideElementsBy<float>(scalar);}
template <>
Array& Array::operator/=(const double& scalar) { return this->divideElementsBy<double>(scalar);}

template <>
Array& Array::operator/=(const Array& other) {
    if      (hasDataOfType<int8_t>())   { return this->divideElementsFrom<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->divideElementsFrom<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->divideElementsFrom<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->divideElementsFrom<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->divideElementsFrom<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->divideElementsFrom<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->divideElementsFrom<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->divideElementsFrom<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->divideElementsFrom<float>(other);}
    else if (hasDataOfType<double>())   { return this->divideElementsFrom<double>(other);}
    else { throw std::runtime_error("Array:: operator/= unsupported array data type"); }
}

template <typename T>
Array& Array::divideElementsFrom(const Array& other) {
   this->must().haveSameSizeAs<Array>(other);
   other.must().haveDataOfType<T>();
        
    if (this->isContiguous()) {
        if (other.isContiguous()) {
            return this->divideElementsOfThisContiguousUsingContiguous<T>(other);
        } else {
            return this->divideElementsOfThisContiguousUsingNonContiguous<T>(other);
        }
    
    } else {
        if (other.isContiguous()) {
            return this->divideElementsOfThisNonContiguousUsingContiguous<T>(other);
        } else {
            return this->divideElementsOfThisNonContiguousUsingNonContiguous<T>(other);
        }
    }

    throw std::runtime_error("theoretically unreachable, bad conditioning");
}

template <typename T>
Array& Array::divideElementsOfThisContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    for (size_t i = 0; i < this->_size; i++) {
        thisData[i] /= otherData[i];
    }
    
    return *this;
}

template <typename T>
Array& Array::divideElementsOfThisContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = other.getOffsetFromFlatIndex<T>(i);
        thisData[i] /= otherData[offset];
    }

    return *this;
}

template <typename T>
Array& Array::divideElementsOfThisNonContiguousUsingContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t offset;
    for (size_t i = 0; i < this->_size; i++) {
        offset = this->getOffsetFromFlatIndex<T>(i);
        thisData[offset] /= otherData[i];
    }

    return *this;
}

template <typename T>
Array& Array::divideElementsOfThisNonContiguousUsingNonContiguous(const Array& other) {    
    T* thisData = this->getPointerOfModifiableDataFast<T>();
    const T* otherData = other.getPointerOfReadOnlyDataFast<T>();

    size_t thisOffset;
    size_t otherOffset;
    for (size_t i = 0; i < this->_size; i++) {
        thisOffset = this->getOffsetFromFlatIndex<T>(i);
        otherOffset = other.getOffsetFromFlatIndex<T>(i);
        thisData[thisOffset] /= otherData[otherOffset];
    }

    return *this;
}


