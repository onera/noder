# include "array/array.hpp"
# include "array/assertions.hpp"
# include "array/factory/strings.hpp"

Array::Array() {
    this->setArrayMembersAsNull();
}


void Array::setArrayMembersAsNull() {
    this->pyArray = py::none();
    this->_dimensions = 0;
    this->_size = 0;
    this->_must = nullptr;
}


// Array::Array(py::none none) {
//     this->setArrayMembersAsNull();
// }


Array::Array(const py::array& other) {
    this->setArrayMembersUsing(other);
}


std::vector<size_t> castToUnsignedVector(const std::vector<ssize_t>& signedVec) {
    std::vector<size_t> unsignedVec(signedVec.size());

    std::transform(signedVec.begin(), signedVec.end(), unsignedVec.begin(),
                [](ssize_t val) -> size_t {
                    if (val < 0) {
                        throw std::overflow_error("Cannot convert negative ssize_t to size_t");
                    }
                    return static_cast<size_t>(val);
                });
    return unsignedVec;
}

void Array::setArrayMembersUsing(const py::array& pyarray) {
    py::buffer_info info = pyarray.request();
    this->pyArray = pyarray;
    this->_dimensions = static_cast<size_t>(info.ndim);
    this->_size = static_cast<size_t>(info.size);
    this->_shape = castToUnsignedVector(info.shape);
    this->_strides = castToUnsignedVector(info.strides);
    this->_must = nullptr;
}




Array::Array(const std::string& str) {
    Array array = arrayfactory::arrayFromString(str);
    this->setArrayMembersUsing(array.pyArray);
}

Array::Array(const char* str) {
    Array array = arrayfactory::arrayFromString(std::string(str));
    this->setArrayMembersUsing(array.pyArray);
}


Array::Array(const Array& other) : pyArray(other.pyArray) {
    this->setArrayMembersUsing(pyArray);
}


// TODO : reduce this boilerplate code
Array::Array(int8_t scalar) {
    py::array arr = py::array_t<int8_t>({1});
    auto buffer = arr.mutable_unchecked<int8_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(int16_t scalar) {
    py::array arr = py::array_t<int16_t>({1});
    auto buffer = arr.mutable_unchecked<int16_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(int32_t scalar) {
    py::array arr = py::array_t<int32_t>({1});
    auto buffer = arr.mutable_unchecked<int32_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(int64_t scalar) {
    py::array arr = py::array_t<int64_t>({1});
    auto buffer = arr.mutable_unchecked<int64_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(uint8_t scalar) {
    py::array arr = py::array_t<uint8_t>({1});
    auto buffer = arr.mutable_unchecked<uint8_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(uint16_t scalar) {
    py::array arr = py::array_t<uint16_t>({1});
    auto buffer = arr.mutable_unchecked<uint16_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(uint32_t scalar) {
    py::array arr = py::array_t<uint32_t>({1});
    auto buffer = arr.mutable_unchecked<uint32_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(uint64_t scalar) {
    py::array arr = py::array_t<uint64_t>({1});
    auto buffer = arr.mutable_unchecked<uint64_t>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(float scalar) {
    py::array arr = py::array_t<float>({1});
    auto buffer = arr.mutable_unchecked<float>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(double scalar) {
    py::array arr = py::array_t<double>({1});
    auto buffer = arr.mutable_unchecked<double>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}

Array::Array(bool scalar) {
    py::array arr = py::array_t<bool>({1});
    auto buffer = arr.mutable_unchecked<bool>();
    buffer(0) = scalar;
    this->setArrayMembersUsing(arr);
}



Array::Assertions& Array::must() const {
    if (!_must) {
        _must = std::make_shared<Assertions>(*this);  // Lazy initialization
    }
    return *_must;
}

std::shared_ptr<Data> Array::clone() const {
    return std::make_shared<Array>(*this);
}


bool Array::hasString() const {
    py::dtype dtype = pyArray.dtype();
    char kind = dtype.kind();
    return (kind == 'U' || kind == 'S');
}


bool Array::isNone() const {
    auto buffer = pyArray.request();
    auto ptr = static_cast<py::object *>(buffer.ptr);
    ssize_t size = buffer.size;

    if (size < 1) {
        return true;
    } else if (size == 1) {
        return ptr[0].is_none();
    }
    return false;
}


bool Array::isScalar() const {
    if (this->isNone()) return false;
    if (this->hasString()) return false;
    if (this->size() == 1) return true;
    return false;
}


bool Array::isContiguous() const {
    return this->isContiguousInStyleC() || this->isContiguousInStyleFortran();
}


bool Array::isContiguousInStyleC() const {
    std::vector<size_t> strides = this->strides();
    std::vector<size_t> shape = this->shape();
    size_t expectedStride = static_cast<size_t>(pyArray.itemsize());
    size_t dims = this->dimensions();

    if (dims == 0) return true;

    for (size_t j = 0; j < dims; ++j) {
        size_t dim = dims - 1 - j;
        if (strides[dim] != expectedStride) {
            return false;
        }
        size_t currentShape = shape[dim];
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
    size_t expectedStride = static_cast<size_t>(pyArray.itemsize());

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
    py::dtype dtype = pyArray.dtype();
    char kind = dtype.kind();
    std::string str_dtype = std::string(py::str(dtype));    

    if (kind == 'S' || kind == 'U') {
        txt += "Array dtype: " + str_dtype + "\n";
        txt += this->extractString();
    }
    
    else {
        txt += "Array dtype: " + str_dtype + " ("+
               std::string(1,dtype.byteorder()) + 
               std::string(1,kind) + 
               std::to_string(dtype.itemsize())+")\n";
        if (pyArray.owndata()) {
            txt += "owns its data\n";
        } else {
            txt += "data is a view\n";
        }
        if (pyArray.flags() & py::array::c_style) {
            txt += "C contiguous\n";
        } else {
            txt += "C non-contiguous\n";
        }
        if (pyArray.flags() & py::array::f_style) {
            txt += "Fortran contiguous\n";
        } else {
            txt += "Fortran non-contiguous\n";
        }
        txt += "shape: ";
        ssize_t total_npts = 1;
        for (ssize_t dim = 0; dim < pyArray.ndim(); dim++)
        {
            if (dim>0) txt += "x";
            ssize_t shape_dim = pyArray.shape(dim);
            total_npts *= shape_dim;
            txt += std::to_string(shape_dim);
        }
        txt += " ="+ std::to_string(total_npts) +"\n";
        
        txt += this->getPrintString();
    }
    return txt;

}

std::string Array::shortInfo() const {

    std::string txt;
    py::dtype dtype = pyArray.dtype();
    char kind = dtype.kind();
    std::string str_dtype = std::string(py::str(dtype));    

    txt += "Array " + str_dtype + " ("+
            std::string(1,dtype.byteorder()) + 
            std::string(1,kind) + 
            std::to_string(dtype.itemsize())+") ";

    size_t sizeOfArray = this->size();
    
    if ( sizeOfArray < 7 ) {
        txt += this->getPrintString(30);
    }
    
    return txt;
}

/*
    template instantiations
*/


template <typename ArrayType, typename T, size_t... DIMS>
void instantiateAccessors(std::index_sequence<DIMS...>) {
    (utils::forceSymbol(&ArrayType::template getAccessorOfReadOnlyData<T, static_cast<ssize_t>(DIMS)>), ...);
    (utils::forceSymbol(&ArrayType::template getAccessorOfModifiableData<T, static_cast<ssize_t>(DIMS)>), ...);
}

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        constexpr size_t MaxDims = 10; // Adjust this for higher dimensions
        constexpr auto dims = std::make_index_sequence<MaxDims>{}; // Generate [0, 1, ..., MaxDims-1]

        // Loop through all types and instantiate accessors for all dimensions
        (instantiateAccessors<Array, U>(dims), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

template <typename... T>
struct InstantiatorMethodScalar {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&Array::template hasDataOfType<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorMethodScalar, utils::ScalarTypes>();
