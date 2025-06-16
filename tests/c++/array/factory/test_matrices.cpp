# include "test_matrices.hpp"

using namespace arrayfactory;


/*
    testing of function zeros
*/

template <typename T>
void test_zeros_c_order() {
    std::vector<size_t> shape = {3, 2};
    Array array = zeros<T>(shape, 'C');
    if (array.size() != 6) {
        throw py::value_error("zeros C-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],static_cast<T>(0))) {
            throw py::value_error("zeros C-order: non-zero value found at index " + std::to_string(i));
        }
    }

    // Check C-order strides for [3,2]: {2*sizeof(T), sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(2 * sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(sizeof(T))) {
        throw py::value_error("zeros C-order: incorrect strides");
    }
}

template <typename T>
void test_zeros_f_order() {
    std::vector<size_t> shape = {3, 2};
    Array array = zeros<T>(shape, 'F');
    if (array.size() != 6) {
        throw py::value_error("zeros F-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],static_cast<T>(0))) {
            throw py::value_error("zeros F-order: non-zero value found at index " + std::to_string(i));
        }
    }

    // Check F-order strides for [3,2]: {sizeof(T), 3*sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(3 * sizeof(T))) {
        throw py::value_error("zeros F-order: incorrect strides");
    }
}

/*
    testing of function ones
*/
template <typename T>
void test_ones_c_order() {
    std::vector<size_t> shape = {2, 3};
    Array array = ones<T>(shape, 'C');
    if (array.size() != 6) {
        throw py::value_error("ones C-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],static_cast<T>(1))) {
            throw py::value_error("ones C-order: non-one value found at index " + std::to_string(i));
        }
    }

    // Check C-order strides for [2,3]: {3*sizeof(T), sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(3 * sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(sizeof(T))) {
        throw py::value_error("ones C-order: incorrect strides");
    }
}

template <typename T>
void test_ones_f_order() {
    std::vector<size_t> shape = {2, 3};
    Array array = ones<T>(shape, 'F');
    if (array.size() != 6) {
        throw py::value_error("ones F-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],static_cast<T>(1))) {
            throw py::value_error("ones F-order: non-one value found at index " + std::to_string(i));
        }
    }

    // Check F-order strides for [2,3]: {sizeof(T), 2*sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(2 * sizeof(T))) {
        throw py::value_error("ones F-order: incorrect strides");
    }
}

/*
    testing function full
*/

template <typename T>
void test_full_c_order() {
    std::vector<size_t> shape = {2, 2};
    T fill_val = static_cast<T>(42);
    Array array = full(shape, fill_val, 'C');
    if (array.size() != 4) {
        throw py::value_error("full C-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],fill_val)) {
            throw py::value_error("full C-order: incorrect value at index " + std::to_string(i));
        }
    }

    // Check strides for C-order (shape [2,2]): {2*sizeof(T), sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(2 * sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(sizeof(T))) {
        throw py::value_error("full C-order: incorrect strides");
    }
}

template <typename T>
void test_full_f_order() {
    std::vector<size_t> shape = {2, 2};
    T fill_val = static_cast<T>(-1);
    Array array = full(shape, fill_val, 'F');
    if (array.size() != 4) {
        throw py::value_error("full F-order: wrong size");
    }

    T* data = array.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < array.size(); ++i) {
        if (!utils::approxEqual(data[i],fill_val)) {
            throw py::value_error("full F-order: incorrect value at index " + std::to_string(i));
        }
    }

    // Check strides for F-order (shape [2,2]): {sizeof(T), 2*sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(2 * sizeof(T))) {
        throw py::value_error("full F-order: incorrect strides");
    }
}


/*
    testing of empty
*/

template <typename T>
void test_empty_c_order() {
    std::vector<size_t> shape = {2, 3};
    Array array = empty<T>(shape, 'C');
    if (array.size() != 2 * 3) {
        throw py::value_error("empty C-order: wrong size");
    }

    // Check C-order strides: For shape [2,3], strides should be {3*sizeof(T), sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(3 * sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(sizeof(T))) {
        throw py::value_error("empty C-order: incorrect strides");
    }
}


template <typename T>
void test_empty_f_order() {
    std::vector<size_t> shape = {2, 3};
    Array array = empty<T>(shape, 'F');
    if (array.size() != 2 * 3) {
        throw py::value_error("empty F-order: wrong size");
    }

    
    // Check F-order strides: For shape [2,3], strides should be {sizeof(T), 2*sizeof(T)}
    if (array.strides()[0] != static_cast<py::ssize_t>(sizeof(T)) || array.strides()[1] != static_cast<py::ssize_t>(2 * sizeof(T))) {
        throw py::value_error("empty F-order: incorrect strides");
    }
}


/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_zeros_c_order<U>()), ...);
        (static_cast<void>(test_zeros_f_order<U>()), ...);
        (static_cast<void>(test_ones_c_order<U>()), ...);
        (static_cast<void>(test_ones_f_order<U>()), ...);
        (static_cast<void>(test_full_c_order<U>()), ...);
        (static_cast<void>(test_full_f_order<U>()), ...);
        (static_cast<void>(test_empty_c_order<U>()), ...);
        (static_cast<void>(test_empty_f_order<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::FloatingAndIntegralTypes>();
