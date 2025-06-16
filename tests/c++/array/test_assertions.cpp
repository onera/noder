# include "test_assertions.hpp"

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_mustHaveDataOfTypeAndDimensions<U>()), ...);
        (static_cast<void>(test_mustHaveDataOfType<U>()), ...);
        (static_cast<void>(test_mustHaveValidDataTypeForSettingScalar<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

// code

void test_assertSameSizeAsVector() {
    Array array = arrayfactory::uniformFromStep<int32_t>(1,5,1);

    {
        auto other = std::vector<float>({1,2,3,4});
        array.must().haveSameSizeAs(other);
    }

    {
        auto other = std::vector<float>({1,2,3});
        try {
            array.must().haveSameSizeAs(other);
        } catch (const py::value_error& e) {
            std::string expectedMessage = "Array size (4) was not equal to 3";
            assert(std::string(e.what()) == expectedMessage && "Incorrect error message");
            std::cout << "Test passed: Caught expected error: " << e.what() << std::endl;
        }   
    }
}

void test_assertSameSizeAsArray() {
    Array array = arrayfactory::uniformFromStep<int32_t>(1,5,1);

    {
        auto other = arrayfactory::uniformFromStep<int8_t>(1,5,1);
        array.must().haveSameSizeAs(other);
    }

    {
        auto other = arrayfactory::uniformFromStep<int8_t>(1,4,1);
        try {
            array.must().haveSameSizeAs(other);
        } catch (const py::value_error& e) {
            std::string expectedMessage = "Array size (4) was not equal to 3";
            assert(std::string(e.what()) == expectedMessage && "Incorrect error message");
            std::cout << "Test passed: Caught expected error: " << e.what() << std::endl;
        }   
    }
}


void test_assertSameSizeAsPyArray() {
    Array array = arrayfactory::uniformFromStep<int32_t>(1,5,1);

    {
        std::vector<int> vector = {1, 2, 3, 4};
        auto other = py::array_t<int>({4}, vector.data());
        array.must().haveSameSizeAs(other);
    }

    {
        std::vector<int> vector = {1, 2, 3};
        auto other = py::array_t<int>({3}, vector.data());
        
        try {
            array.must().haveSameSizeAs(other);
        } catch (const py::value_error& e) {
            std::string expectedMessage = "Array size (4) was not equal to 3";
            assert(std::string(e.what()) == expectedMessage && "Incorrect error message");
            std::cout << "Test passed: Caught expected error: " << e.what() << std::endl;
        }
    }
}

template <typename T>
void test_mustHaveDataOfTypeAndDimensions() {
    {
        Array array = arrayfactory::zeros<T>({3});
        array.must().haveDataOfTypeAndDimensions<T,1>();
    }

    {
        Array array = arrayfactory::zeros<T>({3,3});
        array.must().haveDataOfTypeAndDimensions<T,2>();
    }

    {
        Array array = arrayfactory::zeros<T>({3,3,3});
        array.must().haveDataOfTypeAndDimensions<T,3>();
    }
}


template <typename T>
void test_mustHaveDataOfType() {
    Array array = arrayfactory::zeros<T>({3});
    array.must().haveDataOfType<T>();
}


void test_mustHaveDataOfTypeCatchExpectedError() {
    try {
        Array array = arrayfactory::zeros<int>({3});
        array.must().haveDataOfType<float>();
    } catch (const py::type_error& e) {
        std::string expectedErrorMessageStart = "Wrong requested type";
        if ( !utils::stringStartsWith(e.what(), expectedErrorMessageStart) ) {
            throw e;
        }
    }
}


void test_mustHaveDataOfDimensions() {
    {
        Array array = arrayfactory::zeros<bool>({2});
        array.must().haveDataOfDimensions<1>();
    }

    {
        Array array = arrayfactory::zeros<bool>({2,2});
        array.must().haveDataOfDimensions<2>();
    }

    {
        Array array = arrayfactory::zeros<bool>({2,2,2});
        array.must().haveDataOfDimensions<3>();
    }
}


void test_mustHaveDataOfDimensionsCatchExpectedError() {
    Array array = arrayfactory::zeros<bool>({2});
 
    try {
        array.must().haveDataOfDimensions<2>();
    } catch (const py::type_error& e) {
        std::string expectedErrorMessageStart = "Expected dimensions";
        if ( !utils::stringStartsWith(e.what(), expectedErrorMessageStart) ) {
            throw e;
        }
    }
}


template <typename T>
void test_mustHaveValidDataTypeForSettingScalar() {
    Array array = arrayfactory::zeros<T>({3});
    array.must().haveValidDataTypeForSettingScalar<T>();
}

void test_mustHaveValidDataTypeForSettingScalarCatchExpectedError() {
    Array array("a string");

    try {
        array.must().haveValidDataTypeForSettingScalar<int>();
    } catch (const py::type_error& e) {
        if (std::string(e.what()) != std::string("cannot assign a scalar to an array containing a string")) {
            throw e;
        }
    }
}


