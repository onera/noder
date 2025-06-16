# include "array/array.hpp"


namespace py = pybind11;

void Array::print(const size_t& maxChars) const {
    std::cout << getPrintString(maxChars) << std::endl;
}

std::string Array::getPrintString(const size_t& max_chars) const {
    
    if (this->isNone()) return std::string("None");
    
    std::string txt;
    
    if (this->hasString()) {
        txt = this->extractString();
    } else {
        txt = this->getNumericalArrayPrintDispatchingByType();
    }

    return utils::clipStringIfTooLong(txt, max_chars);
}

std::string Array::getNumericalArrayPrintDispatchingByType() const {
    if      (hasDataOfType<int8_t>())   { return this->getNumericalArrayPrint<int8_t>();}
    else if (hasDataOfType<int16_t>())  { return this->getNumericalArrayPrint<int16_t>();}
    else if (hasDataOfType<int32_t>())  { return this->getNumericalArrayPrint<int32_t>();}
    else if (hasDataOfType<int64_t>())  { return this->getNumericalArrayPrint<int64_t>();}
    else if (hasDataOfType<uint8_t>())  { return this->getNumericalArrayPrint<uint8_t>();}
    else if (hasDataOfType<uint16_t>()) { return this->getNumericalArrayPrint<uint16_t>();}
    else if (hasDataOfType<uint32_t>()) { return this->getNumericalArrayPrint<uint32_t>();}
    else if (hasDataOfType<uint64_t>()) { return this->getNumericalArrayPrint<uint64_t>();}
    else if (hasDataOfType<float>())    { return this->getNumericalArrayPrint<float>();}
    else if (hasDataOfType<double>())   { return this->getNumericalArrayPrint<double>();}
    else if (hasDataOfType<bool>())     { return this->getNumericalArrayPrint<bool>();}
    else { throw std::runtime_error("getNumericalArrayPrintDispatchingByType: unsupported array data type"); }
}


template <typename T>
std::string Array::getNumericalArrayPrint() const {

    std::stringstream ss;

    ss << "[ ";
    for (size_t i = 0; i < this->size(); ++i) {
        T value = this->getItemAtIndex<T>(i);
        ss << static_cast<double>(value) << " ";
    }
    ss << "]" << std::endl;

    return ss.str();
}
template std::string Array::getNumericalArrayPrint<int8_t>() const;
template std::string Array::getNumericalArrayPrint<int16_t>() const;
template std::string Array::getNumericalArrayPrint<int32_t>() const;
template std::string Array::getNumericalArrayPrint<int64_t>() const;
template std::string Array::getNumericalArrayPrint<uint8_t>() const;
template std::string Array::getNumericalArrayPrint<uint16_t>() const;
template std::string Array::getNumericalArrayPrint<uint32_t>() const;
template std::string Array::getNumericalArrayPrint<uint64_t>() const;
template std::string Array::getNumericalArrayPrint<float>() const;
template std::string Array::getNumericalArrayPrint<double>() const;
template std::string Array::getNumericalArrayPrint<bool>() const;
