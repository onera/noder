# include "test_array_data_accessors.hpp"

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

    Array array = Array(py::array_t<T>({3, 3}, expected.data()));
    
    auto data = array.getAccessorOfReadOnlyData<T,2>();

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

    Array array = Array(py::array_t<int>({3, 3}, expected.data()));
    
    auto data = array.getAccessorOfModifiableData<int,2>();

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


/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_scalarSlicingProducesScalar<U>()), ...);
        (static_cast<void>(test_scalarSlicingDoesNotMakeCopy<U>()), ...);
        (static_cast<void>(test_getItemAtIndex<U>()), ...);
        (static_cast<void>(test_getPointerOfDataSafely<U>()), ...);
        (static_cast<void>(test_getPointerOfModifiableDataFast<U>()), ...);
        (static_cast<void>(test_getPointerOfReadOnlyDataFast<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

template <typename... T>
struct InstantiatorAccessors {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_getAccessorOfReadOnlyData<U>()), ...);
        (static_cast<void>(test_getAccessorOfModifiableData<U>()), ...);
        (utils::forceSymbol(&Array::template getAccessorOfReadOnlyData<U, 2>), ...);
        (utils::forceSymbol(&Array::template getAccessorOfModifiableData<U, 2>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorAccessors,
                                             utils::FloatingAndIntegralTypes>();
