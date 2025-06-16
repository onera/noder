# include "test_comparisons.hpp"

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(test_twoIdenticalArraysAreEqual<U>()), ...);
        (static_cast<void>(test_twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual<U>()), ...);
        (static_cast<void>(test_twoArraysWithDifferentItemsAreNotEqual<U>()), ...);
        (static_cast<void>(test_twoArraysWithDifferentSizesAreNotEqual<U>()), ...);
        (static_cast<void>(test_twoArraysWithDifferentShapesButSameSizeAreEqual<U>()), ...);
        (static_cast<void>(test_arrayOfZerosIsEqualToScalarZero<U>()), ...);
        (static_cast<void>(test_arrayOfZerosIsDifferentToScalarOne<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();

// code

template <typename T>
void test_twoIdenticalArraysAreEqual() {
    Array array1 = arrayfactory::zeros<T>({3,3,3});
    Array array2 = arrayfactory::zeros<T>({3,3,3});

    assertEqualArraysAndNotDifferent(array1, array2);
}


template <typename T>
void test_twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual() {
    Array array1 = arrayfactory::zeros<T>({3,3,3});
    Array array2 = arrayfactory::zeros<T>({3,3,3},'F');

    assertEqualArraysAndNotDifferent(array1, array2);
}


void test_twoIdenticalArraysButWithDifferentDataTypesAreEqual() {
    Array array1 = arrayfactory::zeros<int>({3,3,3});
    Array array2 = arrayfactory::zeros<float>({3,3,3});

    assertEqualArraysAndNotDifferent(array1, array2);
}


template <typename T>
void test_twoArraysWithDifferentItemsAreNotEqual() {
    Array array1 = arrayfactory::zeros<T>({3,3,3});
    Array array2 = arrayfactory::ones<T>({3,3,3});

    assertDifferentArraysAndNotEqual(array1, array2);
}


template <typename T>
void test_twoArraysWithDifferentSizesAreNotEqual() {
    Array array1 = arrayfactory::zeros<T>({3});
    Array array2 = arrayfactory::zeros<T>({4});

    assertDifferentArraysAndNotEqual(array1, array2);
}


template <typename T>
void test_twoArraysWithDifferentShapesButSameSizeAreEqual() {
    Array array1 = arrayfactory::zeros<T>({9});
    Array array2 = arrayfactory::zeros<T>({3,3});

    assertEqualArraysAndNotDifferent(array1, array2);
}


template <typename T>
void test_arrayOfZerosIsEqualToScalarZero() {
    Array array = arrayfactory::zeros<T>({3,3,3});

    assertArrayEqualAndNotDifferentToScalar<T>(array, 0);
}


template <typename T>
void test_arrayOfZerosIsDifferentToScalarOne() {
    Array array = arrayfactory::zeros<T>({3,3,3});

    assertArrayDifferentAndNotEqualToScalar<T>(array,1);
}

void test_rangeIsNeverEqualToScalar() {
    Array array = arrayfactory::uniformFromStep<int16_t>(0,5);

    for (size_t i = 0; i < array.size()+3; i++) {
        int16_t scalar = static_cast<int16_t>(i);
        assertArrayDifferentAndNotEqualToScalar<int16_t>(array, scalar);
    }
}

void test_twoIdenticalArraysContainingStringsAreEqual() {
    Array array1("test string");
    Array array2("test string");

    assertEqualArraysAndNotDifferent(array1, array2);
}

void test_twoIdenticalArraysContainingUnicodeStringsAreEqual() {
    Array array1("ρω");
    Array array2("ρω");

    assertEqualArraysAndNotDifferent(array1, array2);
}

void test_arrayEqualToString() {
    Array array("test");

    bool isArrayEqualToString = array == std::string("test");
    bool isArrayDifferentToString = array != std::string("test");

    if (!isArrayEqualToString) {
        throw py::value_error("expected equal to string");
    } else if (isArrayDifferentToString) {
        throw py::value_error("expected not different to string");
    }
}

void test_arrayEqualToUnicodeString() {
    Array array("ρω");

    bool isArrayEqualToString = array == std::string("ρω");
    bool isArrayDifferentToString = array != std::string("ρω");

    if (!isArrayEqualToString) {
        throw py::value_error("expected equal to string");
    } else if (isArrayDifferentToString) {
        throw py::value_error("expected not different to string");
    }
}

void test_arrayDifferentToString() {
    Array array("ρω");

    bool isArrayEqualToString = array == std::string("toto");
    bool isArrayDifferentToString = array != std::string("toto");

    if (isArrayEqualToString) {
        throw py::value_error("expected not equal to string");
    } else if (!isArrayDifferentToString) {
        throw py::value_error("expected different to string");
    }
}

void test_numericalArrayDifferentToString() {
    Array array = arrayfactory::zeros<int>({2,2});

    bool isArrayEqualToString = array == std::string("toto");
    bool isArrayDifferentToString = array != std::string("toto");

    if (isArrayEqualToString) {
        throw py::value_error("expected not equal to string");
    } else if (!isArrayDifferentToString) {
        throw py::value_error("expected different to string");
    }
}


void assertEqualArraysAndNotDifferent(const Array& array1, const Array& array2) {
    bool arraysAreEqual = array1 == array2;
    bool arraysAreDifferent = array1 != array2;

    if (!arraysAreEqual) {
        throw py::value_error("expected equal arrays: failed operator==");
    } else if (arraysAreDifferent) {
        throw py::value_error("expected equal arrays: failed operator!=");
    }
}

void assertDifferentArraysAndNotEqual(const Array& array1, const Array& array2) {
    bool arraysAreEqual = array1 == array2;
    bool arraysAreDifferent = array1 != array2;
    
    if (arraysAreEqual) {
        throw py::value_error("expected different arrays: failed operator==");
    } else if (!arraysAreDifferent) {
        throw py::value_error("expected different arrays: failed operator!=");
    }
}

template <typename T>
void assertArrayEqualAndNotDifferentToScalar(const Array& array1, const T& scalar) {
    bool arrayIsEqualToScalar = array1 == scalar;
    bool arrayIsDifferentToScalar = array1 != scalar;

    if (!arrayIsEqualToScalar) {
        throw py::value_error("expected array equal to scalar: failed operator==");
    } else if (arrayIsDifferentToScalar) {
        throw py::value_error("expected array equal to scalar: failed operator!=");
    }
}


template <typename T>
void assertArrayDifferentAndNotEqualToScalar(const Array& array1, const T& scalar) {
    bool arrayIsEqualToScalar = array1 == scalar;
    bool arrayIsDifferentToScalar = array1 != scalar;

    if (arrayIsEqualToScalar) {
        throw py::value_error("expected array different to scalar: failed operator==");
    } else if (!arrayIsDifferentToScalar) {
        throw py::value_error("expected array different to scalar: failed operator!=");
    }
}


