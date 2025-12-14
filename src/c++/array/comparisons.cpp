# include "array/array.hpp"


bool Array::operator==(const Array& other) const {
    
    if      (hasDataOfType<int8_t>())   { return this->hasAllItemsEqualToThoseIn<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->hasAllItemsEqualToThoseIn<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->hasAllItemsEqualToThoseIn<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->hasAllItemsEqualToThoseIn<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->hasAllItemsEqualToThoseIn<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->hasAllItemsEqualToThoseIn<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->hasAllItemsEqualToThoseIn<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->hasAllItemsEqualToThoseIn<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->hasAllItemsEqualToThoseIn<float>(other);}
    else if (hasDataOfType<double>())   { return this->hasAllItemsEqualToThoseIn<double>(other);}
    else if (hasDataOfType<bool>())     { return this->hasAllItemsEqualToThoseIn<bool>(other);}
    else if (this->hasString()) { return this->hasSameStringAsThatIn(other);}
    else { throw std::runtime_error("Array:: operator== unsupported array data type"); }
}


template <typename T>
bool Array::hasAllItemsEqualToThoseIn(const Array& other) const {
    bool thisIsScalar = this->isScalar();
    bool otherIsScalar = other.isScalar();

    size_t thisSize = this->size();
    size_t otherSize = other.size();

    if (thisIsScalar) {
        auto thisValue = this->getItemAtIndex<T>(0);
        if (otherIsScalar) {
            auto otherValue = this->getItemAtIndex<T>(0);
            return utils::approxEqual(thisValue, otherValue);
        } else {
            for (size_t i = 0; i < otherSize; i++) {
                auto otherValue = other.getItemAtIndex<T>(i);
                if (!utils::approxEqual(thisValue, otherValue)) {
                    return false;
                }
            }
            return true;
        }

    } else if (otherIsScalar) {
        auto otherValue = other.getItemAtIndex<T>(0);
        for (size_t i = 0; i < thisSize; i++) {
            auto thisValue = this->getItemAtIndex<T>(i);
            if (!utils::approxEqual(thisValue, otherValue)) {
                return false;
            }
        }
        return true;

    } else if (thisSize == otherSize) {
        for (size_t i = 0; i < thisSize; i++) {
            const T& thisValue = this->getItemAtIndex<T>(i);
            const T& otherValue = other.getItemAtIndex<T>(i);
            if (!utils::approxEqual(thisValue, otherValue)) {
                return false;
            }
        }
        return true;
    }
    return false;
}



bool Array::operator==(const int8_t& scalar) const { return this->hasAllItemsEqualTo<int8_t>(scalar);}
bool Array::operator==(const int16_t& scalar) const { return this->hasAllItemsEqualTo<int16_t>(scalar);}
bool Array::operator==(const int32_t& scalar) const { return this->hasAllItemsEqualTo<int32_t>(scalar);}
bool Array::operator==(const int64_t& scalar) const { return this->hasAllItemsEqualTo<int64_t>(scalar);}
bool Array::operator==(const uint8_t& scalar) const { return this->hasAllItemsEqualTo<uint8_t>(scalar);}
bool Array::operator==(const uint16_t& scalar) const { return this->hasAllItemsEqualTo<uint16_t>(scalar);}
bool Array::operator==(const uint32_t& scalar) const { return this->hasAllItemsEqualTo<uint32_t>(scalar);}
bool Array::operator==(const uint64_t& scalar) const { return this->hasAllItemsEqualTo<uint64_t>(scalar);}
bool Array::operator==(const float& scalar) const { return this->hasAllItemsEqualTo<float>(scalar);}
bool Array::operator==(const double& scalar) const { return this->hasAllItemsEqualTo<double>(scalar);}
bool Array::operator==(const bool& scalar) const { return this->hasAllItemsEqualTo<bool>(scalar);}
bool Array::operator==(const std::string& otherString) const { return this->hasSameStringAs(otherString);}
bool Array::operator==(const char* otherString) const { return this->hasSameStringAs(std::string(otherString));}

template <typename T>
bool Array::hasAllItemsEqualTo(const T& other) const {
    bool thisIsScalar = this->isScalar();

    if (thisIsScalar) {
        const T& thisValue = this->getItemAtIndex<T>(0);
        return utils::approxEqual(thisValue, other);
    } else {
        for (size_t i = 0; i < this->size(); i++) {
            const T& thisValue = this->getItemAtIndex<T>(i);
            if (!utils::approxEqual(thisValue, other)) {
                return false;
            }
        }
        return true;
    }
    return false;
}


bool Array::operator!=(const Array& other) const {
    if      (hasDataOfType<int8_t>())   { return this->hasAtLeastOneItemDifferentToThoseIn<int8_t>(other);}
    else if (hasDataOfType<int16_t>())  { return this->hasAtLeastOneItemDifferentToThoseIn<int16_t>(other);}
    else if (hasDataOfType<int32_t>())  { return this->hasAtLeastOneItemDifferentToThoseIn<int32_t>(other);}
    else if (hasDataOfType<int64_t>())  { return this->hasAtLeastOneItemDifferentToThoseIn<int64_t>(other);}
    else if (hasDataOfType<uint8_t>())  { return this->hasAtLeastOneItemDifferentToThoseIn<uint8_t>(other);}
    else if (hasDataOfType<uint16_t>()) { return this->hasAtLeastOneItemDifferentToThoseIn<uint16_t>(other);}
    else if (hasDataOfType<uint32_t>()) { return this->hasAtLeastOneItemDifferentToThoseIn<uint32_t>(other);}
    else if (hasDataOfType<uint64_t>()) { return this->hasAtLeastOneItemDifferentToThoseIn<uint64_t>(other);}
    else if (hasDataOfType<float>())    { return this->hasAtLeastOneItemDifferentToThoseIn<float>(other);}
    else if (hasDataOfType<double>())   { return this->hasAtLeastOneItemDifferentToThoseIn<double>(other);}
    else if (hasDataOfType<bool>())     { return this->hasAtLeastOneItemDifferentToThoseIn<bool>(other);}
    else if (this->hasString()) { return this->hasDifferentStringAsThatIn(other);}
    else { throw std::runtime_error("Array:: operator!= unsupported array data type"); }
}


template <typename T>
bool Array::hasAtLeastOneItemDifferentToThoseIn(const Array& other) const {
    bool thisIsScalar = this->isScalar();
    bool otherIsScalar = other.isScalar();

    size_t thisSize = this->size();
    size_t otherSize = other.size();

    if (thisIsScalar) {
        auto thisValue = this->getItemAtIndex<T>(0);
        if (otherIsScalar) {
            auto otherValue = this->getItemAtIndex<T>(0);
            return !utils::approxEqual(thisValue, otherValue);
        } else {
            for (size_t i = 0; i < otherSize; i++) {
                auto otherValue = other.getItemAtIndex<T>(i);
                if (utils::approxEqual(thisValue, otherValue)) {
                    return false;
                }
            }
            return true;
        }

    } else if (otherIsScalar) {
        auto otherValue = other.getItemAtIndex<T>(0);
        for (size_t i = 0; i < thisSize; i++) {
            auto thisValue = this->getItemAtIndex<T>(i);
            if (utils::approxEqual(thisValue, otherValue)) {
                return false;
            }
        }
        return true;

    } else if (thisSize == otherSize) {
        for (size_t i = 0; i < thisSize; i++) {
            auto thisValue = this->getItemAtIndex<T>(i);
            auto otherValue = other.getItemAtIndex<T>(i);
            if (utils::approxEqual(thisValue, otherValue)) {
                return false;
            }
        }
        return true;
    }
    return true;
}


bool Array::operator!=(const int8_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<int8_t>(scalar);}
bool Array::operator!=(const int16_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<int16_t>(scalar);}
bool Array::operator!=(const int32_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<int32_t>(scalar);}
bool Array::operator!=(const int64_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<int64_t>(scalar);}
bool Array::operator!=(const uint8_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<uint8_t>(scalar);}
bool Array::operator!=(const uint16_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<uint16_t>(scalar);}
bool Array::operator!=(const uint32_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<uint32_t>(scalar);}
bool Array::operator!=(const uint64_t& scalar) const { return this->hasAtLeastOneItemDifferentTo<uint64_t>(scalar);}
bool Array::operator!=(const float& scalar) const { return this->hasAtLeastOneItemDifferentTo<float>(scalar);}
bool Array::operator!=(const double& scalar) const { return this->hasAtLeastOneItemDifferentTo<double>(scalar);}
bool Array::operator!=(const bool& scalar) const { return this->hasAtLeastOneItemDifferentTo<bool>(scalar);}
bool Array::operator!=(const std::string& otherString) const { return this->hasDifferentStringTo(otherString);}
bool Array::operator!=(const char* otherString) const { return this->hasDifferentStringTo(std::string(otherString));}

template <typename T>
bool Array::hasAtLeastOneItemDifferentTo(const T& other) const {
    bool thisIsScalar = this->isScalar();

    if (thisIsScalar) {
        const T& thisValue = this->getItemAtIndex<T>(0);
        return !utils::approxEqual(thisValue, other);
    } else {
        for (size_t i = 0; i < this->size(); i++) {
            const T& thisValue = this->getItemAtIndex<T>(i);
            if (!utils::approxEqual(thisValue, other)) {
                return true;
            }
        }
    }
    return false;
}

bool Array::hasSameStringAsThatIn(const Array& other) const {
    if ( (!this->hasString()) || (!other.hasString()) ) {
        return false;
    }
    return this->extractString() == other.extractString();
}

bool Array::hasDifferentStringAsThatIn(const Array& other) const {
    if ( (!this->hasString()) || (!other.hasString()) ) {
        return true;
    }
    return this->extractString() != other.extractString();
}

bool Array::hasSameStringAs(const std::string& otherString) const {
    if ( !this->hasString() ) {
        return false;
    }
    return this->extractString() == otherString;
}

bool Array::hasDifferentStringTo(const std::string& otherString) const {
    if ( !this->hasString() ) {
        return true;
    }
    return this->extractString() != otherString;
}
