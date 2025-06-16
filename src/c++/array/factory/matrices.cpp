# include "array/factory/matrices.hpp"

namespace arrayfactory {

    namespace py = pybind11;

 
    template <typename T>
    Array full(const std::vector<size_t>& shape, T fill_value, const char order) {

        Array array = empty<T>(shape, order);        
        T* data = array.getPointerOfModifiableDataFast<T>();
        std::fill(data, data + array.size(), fill_value);

        return array;
    }

    template <typename T>
    Array empty(const std::vector<size_t>& shape, const char order) {

        std::vector<size_t> strides;

        if (order == 'C') {
            strides = computeStridesInOrderC<T>(shape);
        
        } else if (order == 'F') {
            strides = computeStridesInOrderF<T>(shape);
        
        } else {
            throw std::invalid_argument("order must be either 'C' or 'F'");
        }
        py::array pyarray = py::array(py::dtype::of<T>(), shape, strides);
        Array array(pyarray);

        return array;
    }

    template <typename T>
    std::vector<size_t> computeStridesInOrderC(const std::vector<size_t>& shape) {
    /*
        C-order: last dimension is contiguous
        Example: shape = [R, C]
        strides for C-order: strides[last] = sizeof(T), 
        and going backwards strides[i] = strides[i+1] * shape[i+1]
    */
    
    size_t stride = sizeof(T);
    size_t dims = shape.size();
    std::vector<size_t> strides(shape.size());
    
    for (size_t j = 0; j < dims; ++j) {
        size_t dim = dims - 1 - j;
        strides[dim] = stride;
        stride *= shape[dim];
    }
    
    return strides;
    }

    template <typename T>
    std::vector<size_t> computeStridesInOrderF(const std::vector<size_t>& shape) {
    /*
        F-order: first dimension is contiguous
        strides for F-order: strides[0] = sizeof(T),
        and going forward strides[i] = strides[i-1] * shape[i-1]
    */
    size_t stride = sizeof(T);
    std::vector<size_t> strides(shape.size());
    
    for (size_t i = 0; i < shape.size(); ++i) {
        strides[i] = stride;
        stride *= shape[i];
    }
    
    return strides;
    }

    template <typename T>
    Array zeros(const std::vector<size_t>& shape, const char order) {
        return full(shape, static_cast<T>(0), order);
    }

    template <typename T>
    Array ones(const std::vector<size_t>& shape, const char order) {
        return full(shape, static_cast<T>(1), order);
    }
}

/*
    templates instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(arrayfactory::zeros<U>(std::vector<size_t>{}, char{})), ...);
        (static_cast<void>(arrayfactory::ones<U>(std::vector<size_t>{}, char{})), ...);
        (static_cast<void>(arrayfactory::empty<U>(std::vector<size_t>{}, char{})), ...);
        (static_cast<void>(arrayfactory::full<U>(std::vector<size_t>{}, double{}, char{})), ...);
        (static_cast<void>(arrayfactory::computeStridesInOrderC<U>(std::vector<size_t>{})), ...);
        (static_cast<void>(arrayfactory::computeStridesInOrderF<U>(std::vector<size_t>{})), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();