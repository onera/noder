# include "test_array.hpp"

void test_constructorEmpty() {
    Array array;
}

void test_constructorPyArray() {
    int rawArray[3] = {1, 2, 3};
    py::array_t<int> pyArray = py::array_t<int>({3}, rawArray);
    Array array(pyArray);
}

void test_constructorString() {
    Array array("test string");
}

void test_constructorAnotherArray() {
    Array other("test string");
    Array array(other);
}

void test_getArrayProperties() {
    Array array = arrayfactory::zeros<int8_t>({3,3});
    
    if (array.dimensions()!=2) {
        py::value_error("array got wrong dimensions");
    }

    if (array.size()!=9) {
        py::value_error("array got wrong size");
    }
    auto shape = array.shape();
    auto strides = array.strides();
}

void test_getFlatIndex() {
    Array array = arrayfactory::zeros<int32_t>({2, 3}, 'C');
    size_t flatIndex = array.getFlatIndex({1, 2});
    if (flatIndex != 5) {
        throw py::value_error("getFlatIndex expected 5");
    }
}

void test_getPyArray() {
    Array array = arrayfactory::zeros<int16_t>({2, 3});
    py::array pyArray = array.getPyArray();
    if (pyArray.ndim() != 2) {
        throw py::value_error("getPyArray expected 2 dimensions");
    }
}

void test_extractString() {
    Array array("example");
    if (array.extractString() != "example") {
        throw py::value_error("extractString expected 'example'");
    }
}

void test_getPrintStringAndInfo() {
    Array array = arrayfactory::uniformFromStep<int32_t>(0, 5);
    array.print(40);
    if (array.getPrintString(40).empty()) {
        throw py::value_error("getPrintString should not be empty");
    }
    if (array.info().empty()) {
        throw py::value_error("info should not be empty");
    }
}


void test_sharingData() {
    int32_t carray[] = {0,0};
    
    Array array0 = arrayfactory::toArray1D(carray,2,false);
    Array array1 = array0;
    Array array2 = Array(array1);
    
    carray[0] = 1;

    std::vector<Array> arrays = {array0, array1, array2};
    for (size_t i = 0; i < arrays.size(); i++)
    {
        Array array = arrays[i];
        int32_t firstElement = array.getItemAtIndex<int32_t>(0);
        if ( firstElement != 1 ) {
            throw py::value_error("array"+std::to_string(i)+" data was not shared, expected 1 but got "+std::to_string(firstElement));
        }
    }
}


void test_arrayWithStringHasStringTrue() {
    Array array("test string");

    if (!array.hasString()) {
        py::value_error("should have detected string in array");
    }
}

void test_arrayWithUnicodeStringHasStringTrue() {
    Array array("Λουίς");

    if (!array.hasString()) {
        py::value_error("should have detected string in array with unicode string");
    }
}

void test_arrayEmptyHasStringFalse() {
    Array array;

    if (array.hasString()) {
        py::value_error("empty array should not have been detected as string");
    }
}

void test_arrayWithNumbersHasStringFalse() {
    Array array = arrayfactory::uniformFromStep<int8_t>(0,5);

    if (array.hasString()) {
        py::value_error("array with numbers should not have been detected as string");
    }
}



void test_isNone() {
    Array array;

    if (!array.isNone()) {
        throw py::value_error("The array should have been None");
    }

}

void test_arrayWithNumbersIsNotNone() {
    Array array = arrayfactory::zeros<int8_t>({2});

    if (array.isNone()) {
        throw py::value_error("array with numbers should not have been None");
    }
}

void test_arrayWithStringIsNotNone() {
    Array array("test");

    if (array.isNone()) {
        throw py::value_error("array with string should not have been None");
    }
}

void test_arrayWithNumberOfSizeZeroIsNone() {
    Array array = arrayfactory::zeros<int8_t>({0});

    if (!array.isNone()) {
        throw py::value_error("zero-size array should have been considered as None");
    }
}

template <typename T>
void test_isScalar() {

    Array nullArray;
    if (nullArray.isScalar()) {
        throw py::value_error("should not have been detected as scalar");
    }

    Array zeroSizeArray = arrayfactory::zeros<T>({0});
    if (zeroSizeArray.isScalar()) {
        throw py::value_error("should not have been detected as scalar");
    }

    Array scalarArray = arrayfactory::zeros<T>({1});
    if (!scalarArray.isScalar()) {
        throw py::value_error("should have been detected as scalar");
    }

    T scalar(1);
    Array directScalarArray = Array(scalar);
    if (!directScalarArray.isScalar()) {
        throw py::value_error("should have been detected as scalar");
    }

    Array vectorArray = arrayfactory::zeros<T>({2});
    if (vectorArray.isScalar()) {
        throw py::value_error("should not have been detected as scalar");
    }

    Array matrixArray = arrayfactory::zeros<T>({3,3});
    if (matrixArray.isScalar()) {
        throw py::value_error("should not have been detected as scalar");
    }

    Array stringArray = arrayfactory::arrayFromString("test string");
    if (stringArray.isScalar()) {
        throw py::value_error("should not have been detected as scalar");
    }

}

template <typename T>
void test_contiguity() {

    {
        Array array = arrayfactory::zeros<T>({3});
        if (!array.isContiguousInStyleC()) {
            throw py::value_error("dim 1 vector should have been detected as C contiguous");
        }
        if (!array.isContiguousInStyleFortran()) {
            throw py::value_error("dim 1 vector should have been detected as F contiguous");
        }
        if (!array.isContiguous()) {
            throw py::value_error("dim 1 vector should have been detected as contiguous");
        }

    }

    {
        Array array = arrayfactory::zeros<T>({3,3}, 'C');
        if (!array.isContiguousInStyleC()) {
            throw py::value_error("dim 2 matrix should have been detected as C contiguous");
        }
        if (array.isContiguousInStyleFortran()) {
            throw py::value_error("dim 2 matrix should not have been detected as F contiguous");
        }
        if (!array.isContiguous()) {
            throw py::value_error("dim 2 matrix should have been detected as contiguous");
        }

    }

    {
        Array array = arrayfactory::zeros<T>({3,3}, 'F');
        if (array.isContiguousInStyleC()) {
            throw py::value_error("dim 2 matrix should not have been detected as C contiguous");
        }
        if (!array.isContiguousInStyleFortran()) {
            throw py::value_error("dim 2 matrix should have been detected as F contiguous");
        }
        if (!array.isContiguous()) {
            throw py::value_error("dim 2 matrix should have been detected as contiguous");
        }

    }

    {
        Array array = arrayfactory::zeros<T>({3,3,3}, 'C');
        if (!array.isContiguousInStyleC()) {
            throw py::value_error("dim 3 matrix should have been detected as C contiguous");
        }
        if (array.isContiguousInStyleFortran()) {
            throw py::value_error("dim 3 matrix should not have been detected as F contiguous");
        }
        if (!array.isContiguous()) {
            throw py::value_error("dim 3 matrix should have been detected as contiguous");
        }

    }

    {
        Array array = arrayfactory::zeros<T>({3,3,3}, 'F');
        if (array.isContiguousInStyleC()) {
            throw py::value_error("dim 3 matrix should not have been detected as C contiguous");
        }
        if (!array.isContiguousInStyleFortran()) {
            throw py::value_error("dim 3 matrix should have been detected as F contiguous");
        }
        if (!array.isContiguous()) {
            throw py::value_error("dim 3 matrix should have been detected as contiguous");
        }

    }


    {
        Array contiguousArray = arrayfactory::zeros<T>({3,3}, 'C');
        py::slice rowSlice(1, 3, 1);
        py::slice colSlice(1, 3, 1);
        //  equivalent to Python's contiguousArray[1:3, 1:3]
        py::array nonContiguousPyArray = contiguousArray.getPyArray()[py::make_tuple(rowSlice, colSlice)];
        Array nonContiguousArray = Array(nonContiguousPyArray);

        if (nonContiguousArray.isContiguous()) {
            throw py::value_error("Expected non-contiguous array");
        }
    }
}

template <typename T>
void test_hasDataOfType() {
    Array array = arrayfactory::zeros<T>({3});
    if (!array.hasDataOfType<T>()) {
        py::value_error("Did not get the expected data type");
    }
}

void test_doNotHaveDataOfType() {
    Array array = arrayfactory::zeros<bool>({3});
    if (array.hasDataOfType<float>()) {
        py::value_error("wrong data type match");
    }
}

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&test_contiguity<U>), ...);
        (utils::forceSymbol(&test_isScalar<U>), ...);
        (utils::forceSymbol(&test_hasDataOfType<U>), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();
