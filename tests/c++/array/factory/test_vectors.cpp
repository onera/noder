# include "test_vectors.hpp"

using namespace arrayfactory;


/*
    testing of uniformFromStep
*/

template <typename T>
void test_uniformFromCount_positive_step() {

    Array result = uniformFromStep<T>(0, 10, 2);
    std::vector<T> expected = {0, 2, 4, 6, 8};
    if (result.size() != expected.size()) {
        throw py::value_error("uniformFromStep failed: wrong size for positive step");
    }

    T* data = result.getPointerOfModifiableDataFast<T>();    
    for (size_t i = 0; i < expected.size(); ++i) {
        std::cout << data[i] << std::endl;
        if (!utils::approxEqual(data[i],expected[i])) {
            throw py::value_error("uniformFromStep failed: incorrect values for positive step");
        }
    }
}

template <typename T>
void test_uniformFromCount_negative_step() {

    Array result = uniformFromStep<T>(10, 0, -2);
    std::vector<T> expected = {10, 8, 6, 4, 2};
    if (result.size() != expected.size()) {
        throw py::value_error("uniformFromStep failed: wrong size for negative step");
    }

    T* data = result.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < expected.size(); ++i) {
        std::cout << data[i] << std::endl;
        if (!utils::approxEqual(data[i],expected[i])) {
            throw py::value_error("uniformFromStep failed: incorrect values for negative step");
        }
    }
}


void test_uniformFromCount_zero_step() {
    try {
        Array result = uniformFromStep<int>(0, 10, 0);
        throw py::value_error("uniformFromStep failed: did not throw exception for zero step");
    } catch (const std::invalid_argument& e) {
        // Expected behavior
    }
}


void test_uniformFromCount_incoherent_step() {
    try {
        Array result = uniformFromStep<int>(10, 0, 1);
        throw py::value_error("uniformFromStep failed: did not throw exception for incoherent step");
    } catch (const std::invalid_argument& e) {
        // Expected behavior
    }
}


/*
    testing of uniformFromCount
*/


template <typename T>
void test_uniformFromCount_endpoint_true() {
    size_t num = 5;
    Array result = uniformFromCount<T>(0, 1, num, true);
    // Expected: [0.0, 0.25, 0.5, 0.75, 1.0]
    if (result.size() != num) {
        throw py::value_error("uniformFromCount endpoint=true: wrong size");
    }

    T* data = result.getPointerOfModifiableDataFast<T>();
    if (!utils::approxEqual(data[0],static_cast<T>(0)) || !utils::approxEqual(data[num-1],static_cast<T>(1))) {
        throw py::value_error("uniformFromCount endpoint=true: incorrect first or last value");
    }
}

template <typename T>
void test_uniformFromCount_endpoint_false() {
    size_t num = 5;
    Array result = uniformFromCount<T>(0, 1, num, false);
    // With endpoint=false, we expect [0, 0.2, 0.4, 0.6, 0.8]
    if (result.size() != num) {
        throw py::value_error("uniformFromCount endpoint=false: wrong size");
    }

    T* data = result.getPointerOfModifiableDataFast<T>();
    if (utils::approxEqual(data[num-1],static_cast<T>(1))) {
        throw py::value_error("uniformFromCount endpoint=false: last value should not be 1.0");
    }
}


template <typename T>
void test_uniformFromCount_num_zero() {

    try {
        Array result = uniformFromCount<T>(0, 1, 0);
    } catch ( const std::invalid_argument& e ) {
        return;
    }
}


template <typename T>
void test_uniformFromCount_num_one() {
    try {
        Array result = uniformFromCount<T>(0, 1, 0);
    } catch ( const std::invalid_argument& e ) {
        return;
    }
}


template <typename T>
void test_uniformFromCount_floating_point_values() {
    size_t num = 5;
    Array result = uniformFromCount<T>(0.0, 2.0, num, true);
    // Expected: [0.0, 0.5, 1.0, 1.5, 2.0]
    T* data = result.getPointerOfModifiableDataFast<T>();
    for (size_t i = 0; i < num; ++i) {
        T expected = static_cast<T>(i) * static_cast<T>(0.5);
        if (!utils::approxEqual(data[i],expected)) {
            throw py::value_error("uniformFromCount floating point: incorrect value at index " + std::to_string(i));
        }
    }
}

/*
    template instantiations
*/


template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_uniformFromCount_positive_step<U>()), ...);
        (static_cast<void>(test_uniformFromCount_negative_step<U>()), ...);
    }
};

template <typename... T>
struct InstantiatorLinspace {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_uniformFromCount_endpoint_true<U>()), ...);
        (static_cast<void>(test_uniformFromCount_endpoint_false<U>()), ...);
        (static_cast<void>(test_uniformFromCount_num_zero<U>()), ...);
        (static_cast<void>(test_uniformFromCount_num_one<U>()), ...);
        (static_cast<void>(test_uniformFromCount_floating_point_values<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::FloatingAndIntegralTypes>();

// Explicit instantiation for test_uniformFromCount
template void utils::instantiate<InstantiatorLinspace, float, double>();

