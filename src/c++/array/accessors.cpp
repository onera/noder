# include "array/array.hpp"


Array Array::operator[](const size_t flatIndex) {

    if      (hasDataOfType<int8_t>())   { return this->getItemAsArrayAtIndex<int8_t>(flatIndex);}
    else if (hasDataOfType<int16_t>())  { return this->getItemAsArrayAtIndex<int16_t>(flatIndex);}
    else if (hasDataOfType<int32_t>())  { return this->getItemAsArrayAtIndex<int32_t>(flatIndex);}
    else if (hasDataOfType<int64_t>())  { return this->getItemAsArrayAtIndex<int64_t>(flatIndex);}
    else if (hasDataOfType<uint8_t>())  { return this->getItemAsArrayAtIndex<uint8_t>(flatIndex);}
    else if (hasDataOfType<uint16_t>()) { return this->getItemAsArrayAtIndex<uint16_t>(flatIndex);}
    else if (hasDataOfType<uint32_t>()) { return this->getItemAsArrayAtIndex<uint32_t>(flatIndex);}
    else if (hasDataOfType<uint64_t>()) { return this->getItemAsArrayAtIndex<uint64_t>(flatIndex);}
    else if (hasDataOfType<float>())    { return this->getItemAsArrayAtIndex<float>(flatIndex);}
    else if (hasDataOfType<double>())   { return this->getItemAsArrayAtIndex<double>(flatIndex);}
    else if (hasDataOfType<bool>())     { return this->getItemAsArrayAtIndex<bool>(flatIndex);}
    else { throw std::runtime_error("Array::operator[] unsupported array data type"); }
}


template <typename T>
Array Array::getItemAsArrayAtIndex(const size_t& flatIndex) {
    T& item = this->getItemAtIndex<T>(flatIndex);
    py::array pyarray(py::dtype::of<T>(),
                        {1},
                        {sizeof(T)},
                        &item,
                        py::none());
    return Array(pyarray);
}


template <typename T>
T& Array::getItemAtIndex(const size_t& index) {
    T* data = this->getPointerOfDataSafely<T>();
    
    if (this->isContiguous()) {
        return data[index];
    
    } else {
        size_t offset = getOffsetFromFlatIndex<T>(index);
        return data[offset];
    }
}


template <typename T>
size_t Array::getOffsetFromFlatIndex(size_t flatIndex) const {
    
    size_t offset = 0;
    size_t mj;
    size_t idx;

    for (size_t j = 0; j < this->_dimensions; ++j) {
        mj = this->_dimensions - 1 - j;
        idx = flatIndex % this->_shape[mj];
        offset += idx * this->_strides[mj] / sizeof(T);
        flatIndex /= this->_shape[mj];
    }

    return offset;
}


template <typename T>
const T& Array::getItemAtIndex(const size_t& index) const {
    const T* data = this->getPointerOfReadOnlyDataFast<T>();

    if (this->isContiguous()) {
        return data[index];
    
    } else {
        size_t offset = getOffsetFromFlatIndex<T>(index);
        return data[offset];
    }
}

template <typename T>
T* Array::getPointerOfDataSafely() {
    must().haveDataOfType<T>();
    py::buffer_info info = this->pyArray.request(); // this has some cost, but it's safe
    return static_cast<T*>(info.ptr);
}


template <typename T>
T* Array::getPointerOfModifiableDataFast() {
    return reinterpret_cast<T*>(this->pyArray.mutable_data()); // this is the fastest, but unsafe
}


template <typename T>
const T* Array::getPointerOfReadOnlyDataFast() const {
    return reinterpret_cast<const T*>(this->pyArray.data()); // this is the fastest, but unsafe
}


size_t Array::getFlatIndex(const std::vector<size_t>& indices) const {

    if ( this->isContiguousInStyleC() ) {
        return this->getFlatIndexOfArrayInStyleC(indices);
    
    } else if ( this->isContiguousInStyleFortran() ) {
        return this->getFlatIndexOfArrayInStyleFortran(indices);
    
    } else {
        // TODO implement for non-contiguous cf getNonContiguousNumericalArrayPrint
        throw std::invalid_argument("getFlatIndex to be implemented for non-contiguous arrays (see getNonContiguousNumericalArrayPrint)");
    }
}


size_t Array::getFlatIndexOfArrayInStyleC(const std::vector<size_t>& indices) const {
    size_t flatIndex = 0;
    size_t multiplier = 1;
    
    if (this->_dimensions == 0) return 0;

    size_t dims = static_cast<size_t>(this->_dimensions);
    size_t currentShape;

    for (size_t j = 0; j < dims; ++j) {
        size_t dim = dims - 1 - j;

        flatIndex += indices[dim] * multiplier;

        currentShape = static_cast<size_t>(this->_shape[dim]); 
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
        multiplier *= static_cast<size_t>(this->_shape[i]);
    }

    return flatIndex;
}

std::string Array::extractString() const {

    char kind = pyArray.dtype().kind();
    if ( kind == 'U' ) {
        return this->extractStringOfKindU();

    } else if (kind=='S') {
        return this->extractStringOfKindS();

    } else {
        return std::string(""); // TODO or throw an exception?
    }
}

std::string Array::extractStringOfKindU() const {

    if (pyArray.dtype().kind() != 'U') {
        throw std::invalid_argument("input array string kind is not 'U'");
    }

    std::string outputString;

    py::buffer_info buffer = pyArray.request();
    
    // Check if the data type size is multiple of 4 (size of one Unicode character in numpy)
    if (buffer.itemsize % 4 != 0) {
        throw std::invalid_argument("Unexpected item size for Unicode array");
    }

    for (ssize_t i = 0; i < pyArray.size(); ++i) {
        std::string separator =  (i==0) ? std::string("") : std::string(" ");

        // Access the element as a PyObject for Unicode string
        auto* dataptr = static_cast<char*>(buffer.ptr) + i * buffer.itemsize;
        py::object pyobj = py::reinterpret_borrow<py::object>(
            PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
                                        dataptr,
                                        buffer.itemsize / 4)
        );
        outputString += separator + pyobj.cast<std::string>();        
    }
    
    return outputString;
}


std::string Array::extractStringOfKindS() const {

    if (pyArray.dtype().kind() != 'S') {
        throw std::invalid_argument("input array string kind is not 'S'");
    }

    std::string outputString;
    py::buffer_info buffer = pyArray.request();
    for (ssize_t i = 0; i < pyArray.size(); ++i) {
        std::string separator =  (i==0) ? std::string("") : std::string(" ");
        auto* data_ptr = static_cast<char*>(buffer.ptr) + i * buffer.itemsize;
        outputString += separator + std::string(data_ptr, static_cast<size_t>(buffer.itemsize));
    }

    return outputString;
}


/*
    template instantiations
*/

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
