# include "test_array_data_accessors.hpp"
# include "array/array_numpy_bridge.hpp"

# include <pybind11/pybind11.h>

namespace py = pybind11;

template <typename T>
void test_scalarSlicingProducesScalar() {
    Array array = arrayfactory::zeros<T>({2});
    
    Array item = array[0];

    if (!item.isScalar()) {
        throw py::value_error("slicing result expected scalar");
    } 
}


template <typename T>
void test_scalarSlicingDoesNotMakeCopy() {
    Array array = arrayfactory::zeros<T>({2});

    T newValue = 1;
    Array item = array[0];
    array[0] = newValue;

    if (item != newValue) {
        throw py::value_error("slicing result should not have made copy");
    } 
}


template <typename T>
void test_getItemAtIndex() {
    T carray[] = {0, 1, 0};
    Array array = arrayfactory::toArray1D(carray,3,false);

    T& firstItem = array.getItemAtIndex<T>(0);

    carray[0] = 1;
    if (!utils::approxEqual(firstItem,carray[0])) {
        throw py::value_error("should not have made copy");
    }
}


template <typename T>
void test_getPointerOfDataSafely() {
    Array array = arrayfactory::zeros<T>({2});
    T* ptr = array.getPointerOfDataSafely<T>();
    ptr[0] = 1;
    ptr[1] = 1;
}


template <typename T>
void test_getPointerOfModifiableDataFast() {
    Array array = arrayfactory::zeros<T>({2});
    T* ptr = array.getPointerOfModifiableDataFast<T>();
    ptr[0] = 1;
    ptr[1] = 1;
}


template <typename T>
void test_getPointerOfReadOnlyDataFast() {
    Array array = arrayfactory::zeros<T>({2});
    const T* ptr = array.getPointerOfReadOnlyDataFast<T>();
    std::cout << ptr[0] << std::endl;
    std::cout << ptr[1] << std::endl;
}

template <typename T>
void test_getAccessorOfReadOnlyData() {

    std::vector<T> expected = {1, 2, 3,
                               4, 5, 6,
                               7, 8, 9};

    Array array = arraybridge::arrayFromPyArray(py::array_t<T>({3, 3}, expected.data()));
    py::array pyArray = arraybridge::toPyArray(array);
    auto data = pyArray.unchecked<T, 2>();

    size_t flatIndex;
    for (size_t i = 0; i < array.shape()[0]; i++)
    {
        for (size_t j = 0; j < array.shape()[1]; j++) {

            flatIndex = array.getFlatIndex({i,j});

            if (!utils::approxEqual(expected[flatIndex],data(i,j))) {
                throw py::value_error("data(" +std::to_string(i)+ "," +std::to_string(j)+ ")=" +std::to_string(data(i,j))+
                    " != expected["+std::to_string(flatIndex)+"]="+std::to_string(expected[flatIndex]));
            }
        }
    }     
}

template <typename T>
void test_getAccessorOfModifiableData() {

    std::vector<int> expected = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9};

    Array array = arraybridge::arrayFromPyArray(py::array_t<int>({3, 3}, expected.data()));
    py::array pyArray = arraybridge::toPyArray(array);
    auto data = pyArray.mutable_unchecked<int, 2>();

    data(1,1) = 0;
    expected[4] = 0;

    size_t flatIndex;
    for (size_t i = 0; i < array.shape()[0]; i++)
    {
        for (size_t j = 0; j < array.shape()[1]; j++) {

            flatIndex = array.getFlatIndex({i,j});

            if (expected[flatIndex] != data(i,j)) {
                throw py::value_error("data(" +std::to_string(i)+ "," +std::to_string(j)+ ")=" +std::to_string(data(i,j))+
                    " != expected["+std::to_string(flatIndex)+"]="+std::to_string(expected[flatIndex]));
            }
        }
    }     
}

void test_getFlatIndexOfArrayInStyleC() {
    Array array = arrayfactory::empty<int8_t>({2,3,4}, 'C');

    {
        std::vector<size_t> indices = {0,0,0};
        size_t expectedIndex = 0;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order C, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }


    {
        std::vector<size_t> indices = {1,2,3};
        size_t expectedIndex = 23;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order C, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }

    {
        std::vector<size_t> indices = {0,1,2};
        size_t expectedIndex = 6;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order C, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }
}


void test_getFlatIndexOfArrayInStyleFortran() {

    Array array = arrayfactory::empty<int8_t>({2,3,4}, 'F');

    {
        std::vector<size_t> indices = {0,0,0};
        size_t expectedIndex = 0;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order F, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }


    {
        std::vector<size_t> indices = {1,2,3};
        size_t expectedIndex = 23;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order F, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }

    {
        std::vector<size_t> indices = {0,1,2};
        size_t expectedIndex = 14;
        size_t resultIndex = array.getFlatIndex(indices);

        if (expectedIndex != resultIndex) {
            throw py::value_error("Test failed for order F, expected "
                + std::to_string(expectedIndex)+"  but got " + std::to_string(resultIndex));
        }
    }
}


void test_extractStringAscii() {
    Array array = arrayfactory::arrayFromString("rho omega");

    std::string extractedString = array.extractString();

    if (extractedString != "rho omega") {
        throw py::value_error("extracted string is not correct");
    }
}


void test_extractStringUnicode() {
    Array arrayWithUnicodeString = arrayfactory::arrayFromUnicodeString("ρω");

    std::string extractedUnicodeString = arrayWithUnicodeString.extractString();
    
    if (extractedUnicodeString != "ρω") {
        throw py::value_error("extracted unicode string is not correct");
    }

}

void test_checkedSpans() {
    Array array = arrayfactory::zeros<double>({3});
    auto writable = array.writableSpan<double>();
    writable[1] = 4.5;

    const Array& readOnlyArray = array;
    const auto values = readOnlyArray.readOnlySpan<double>();
    if (values.size() != 3 || values[1] != 4.5) {
        throw py::value_error("checked spans must share the Array buffer");
    }

    bool wrongTypeRaised = false;
    try {
        (void)array.readOnlySpan<float>();
    } catch (const std::invalid_argument&) {
        wrongTypeRaised = true;
    }
    if (!wrongTypeRaised) {
        throw py::value_error("readOnlySpan must reject a mismatched type");
    }
}

void test_readOnlyArray() {
    py::array_t<double> values({3});
    values.mutable_at(0) = 1.0;
    values.mutable_at(1) = 2.0;
    values.mutable_at(2) = 3.0;
    values.attr("setflags")(false);

    Array array = arraybridge::arrayFromPyArray(values);
    if (array.isWritable()) {
        throw py::value_error("read-only NumPy ownership must be preserved");
    }
    if (array.readOnlySpan<double>()[2] != 3.0) {
        throw py::value_error("read-only span returned unexpected values");
    }

    bool raised = false;
    try {
        (void)array.writableSpan<double>();
    } catch (const std::runtime_error&) {
        raised = true;
    }
    if (!raised) {
        throw py::value_error("writableSpan must reject a read-only Array");
    }
    if (arraybridge::toPyArray(array).writeable()) {
        throw py::value_error("exported NumPy view must remain read-only");
    }
}


/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&test_scalarSlicingProducesScalar<U>), ...);
        (utils::forceSymbol(&test_scalarSlicingDoesNotMakeCopy<U>), ...);
        (utils::forceSymbol(&test_getItemAtIndex<U>), ...);
        (utils::forceSymbol(&test_getPointerOfDataSafely<U>), ...);
        (utils::forceSymbol(&test_getPointerOfModifiableDataFast<U>), ...);
        (utils::forceSymbol(&test_getPointerOfReadOnlyDataFast<U>), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

template <typename... T>
struct InstantiatorAccessors {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&test_getAccessorOfReadOnlyData<U>), ...);
        (utils::forceSymbol(&test_getAccessorOfModifiableData<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorAccessors,
                                             utils::FloatingAndIntegralTypes>();
