# include "test_modifiers.hpp"


template <typename T>
void test_setArrayToScalar() {
    Array array = arrayfactory::zeros<T>({2,2,2});
    T newValue = 1;
    array = newValue;

    if (array!=newValue) {
        throw py::value_error("expected modification of array");
    }
}


void test_catchErrorWhenAssigningWrongScalarType() {
    Array array = arrayfactory::zeros<int32_t>({2,2,2});
    double newValue = 1;

    try {
        array = newValue;
        throw std::runtime_error("should have raised an error");
    
    } catch(const py::type_error& e) {
        std::string expectedErrorMessageStart = "Wrong requested";
        if ( !utils::stringStartsWith(e.what(), expectedErrorMessageStart) ) {
            throw e;
        }
    }
}

void test_catchErrorWhenAssigningScalarToStringArray() {
    Array array("a string");

    try {
        array = 1;
        throw std::runtime_error("should have raised an error");
    
    } catch(const py::type_error& e) {
        std::string expectedErrorMessage = "cannot assign a scalar to an array containing a string";
        if (e.what() != expectedErrorMessage) {
            throw e;
        }
    }
}


template <typename T>
void test_setFromArrayConsideringAllTypes() {
    Array array = arrayfactory::zeros<T>({2,2,2});
    Array other = arrayfactory::ones<T>({2,2,2});
    
    array = other;

    T expectedValue = 1;
    if (array!=expectedValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_setFromArrayToRange() {
    Array array       = arrayfactory::toArray1D(std::vector<double>({2,4,6}));
    Array setArray = arrayfactory::toArray1D(std::vector<double>({1,2,3}));
    Array expected    = arrayfactory::toArray1D(std::vector<double>({1,2,3}));

    array = setArray;
    
    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}


template <typename T>
void test_addScalarConsideringAllTypes() {
    Array array = arrayfactory::zeros<T>({2,2,2});
    T newValue = 1;
    array += newValue;

    if (array!=newValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_addScalarToRange() {
    Array array = arrayfactory::uniformFromStep<int32_t>(0,10);

    int32_t addedValue = 1;
    array += addedValue;

    Array expected = arrayfactory::uniformFromStep<int32_t>(1,11);

    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}


void test_substractScalarToRange() {
    Array array = arrayfactory::uniformFromStep<int32_t>(1,11);

    int32_t substractedValue = 1;
    array -= substractedValue;

    Array expected = arrayfactory::uniformFromStep<int32_t>(0,10);

    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}

template <typename T>
void test_multiplyScalarConsideringAllTypes() {
    Array array = arrayfactory::ones<T>({2,2,2});
    T newValue = 0;
    array *= newValue;

    if (array!=newValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_multiplyScalarToRange() {
    Array array = arrayfactory::uniformFromStep<int32_t>(0,4);

    int32_t multiplyValue = 2;
    array *= multiplyValue;

    Array expected = arrayfactory::toArray1D(std::vector<int32_t>({0,2,4,6}));

    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}

void test_divideScalarToRangeOfIntegers() {
    Array array = arrayfactory::toArray1D(std::vector<int32_t>({2,4,6}));

    int32_t divideValue = 2;
    array /= divideValue;

    Array expected = arrayfactory::toArray1D(std::vector<int32_t>({1,2,3}));

    if (array != expected) {
        py::print(expected);
        py::print(array);
        throw py::value_error("did not get the expected result");
    }
}

void test_divideScalarToRangeOfFloats() {
    Array array = arrayfactory::toArray1D(std::vector<double>({2,4,6}));

    double divideValue = 2;
    array /= divideValue;

    Array expected = arrayfactory::toArray1D(std::vector<double>({1,2,3}));

    if (array != expected) {
        py::print(expected);
        py::print(array);
        throw py::value_error("did not get the expected result");
    }
}


template <typename T>
void test_addFromArrayConsideringAllTypes() {
    Array array = arrayfactory::zeros<T>({2,2,2});
    Array other = arrayfactory::ones<T>({2,2,2});
    
    array += other;

    T expectedValue = 1;
    if (array!=expectedValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_addFromArrayToRange() {
    Array array    = arrayfactory::toArray1D(std::vector<double>({2,4,6}));
    Array addArray = arrayfactory::toArray1D(std::vector<double>({1,2,3}));
    Array expected = arrayfactory::toArray1D(std::vector<double>({3,6,9}));

    array += addArray;
    
    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}


template <typename T>
void test_substractFromArrayConsideringAllTypes() {
    Array array = arrayfactory::ones<T>({2,2,2});
    Array other = arrayfactory::ones<T>({2,2,2});
    
    array -= other;

    T expectedValue = 0;
    if (array!=expectedValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_substractFromArrayToRange() {
    Array array          = arrayfactory::toArray1D(std::vector<double>({2,8,7}));
    Array substractArray = arrayfactory::toArray1D(std::vector<double>({1,2,3}));
    Array expected       = arrayfactory::toArray1D(std::vector<double>({1,6,4}));

    array -= substractArray;
    
    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}


template <typename T>
void test_multiplyFromArrayConsideringAllTypes() {
    Array array = arrayfactory::ones<T>({2,2,2});
    Array other = arrayfactory::zeros<T>({2,2,2});
    
    array *= other;

    T expectedValue = 0;
    if (array!=expectedValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_multiplyFromArrayToRange() {
    Array array         = arrayfactory::toArray1D(std::vector<double>({2,3,3}));
    Array multiplyArray = arrayfactory::toArray1D(std::vector<double>({1,2,3}));
    Array expected      = arrayfactory::toArray1D(std::vector<double>({2,6,9}));

    array *= multiplyArray;
    
    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}



template <typename T>
void test_divideFromArrayConsideringAllTypes() {
    Array array = arrayfactory::ones<T>({2,2,2});
    Array other = arrayfactory::ones<T>({2,2,2});
    
    array /= other;

    T expectedValue = 1;
    if (array!=expectedValue) {
        throw py::value_error("did not get the expected result");
    }
}


void test_divideFromArrayToRange() {
    Array array       = arrayfactory::toArray1D(std::vector<double>({2,4,6}));
    Array divideArray = arrayfactory::toArray1D(std::vector<double>({1,2,2}));
    Array expected    = arrayfactory::toArray1D(std::vector<double>({2,2,3}));

    array /= divideArray;
    
    if (array != expected) {
        throw py::value_error("did not get the expected result");
    }
}


/*
    template instantiations
*/

template <typename... T>
struct InstantiatorScalars {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_setArrayToScalar<U>()), ...);
        (static_cast<void>(test_setFromArrayConsideringAllTypes<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorScalars, utils::ScalarTypes>();

template <typename... T>
struct InstantiatorFloatingAndIntegrals {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_addScalarConsideringAllTypes<U>()), ...);
        (static_cast<void>(test_multiplyScalarConsideringAllTypes<U>()), ...);
        (static_cast<void>(test_addFromArrayConsideringAllTypes<U>()), ...);
        (static_cast<void>(test_substractFromArrayConsideringAllTypes<U>()), ...);
        (static_cast<void>(test_multiplyFromArrayConsideringAllTypes<U>()), ...);
        (static_cast<void>(test_divideFromArrayConsideringAllTypes<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorFloatingAndIntegrals, utils::FloatingAndIntegralTypes>();

