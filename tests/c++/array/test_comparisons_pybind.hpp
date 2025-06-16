# ifndef TEST_ARRAY_COMPARISONS_PYBIND_HPP
# define TEST_ARRAY_COMPARISONS_PYBIND_HPP

# include "test_comparisons.hpp"

void bindTestsOfArrayComparisons(py::module_ &m) {

    utils::bindForScalarTypes(m, "twoIdenticalArraysAreEqual", []<typename T>() { return &test_twoIdenticalArraysAreEqual<T>; });

    utils::bindForScalarTypes(m, "twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual", []<typename T>() { return &test_twoIdenticalArraysButWithDifferentMemoryLayoutAreEqual<T>; });

    utils::bindForScalarTypes(m, "twoArraysWithDifferentItemsAreNotEqual", []<typename T>() { return &test_twoArraysWithDifferentItemsAreNotEqual<T>; });

    m.def("twoIdenticalArraysButWithDifferentDataTypesAreEqual", &test_twoIdenticalArraysButWithDifferentDataTypesAreEqual);

    utils::bindForScalarTypes(m, "twoArraysWithDifferentSizesAreNotEqual", []<typename T>() { return &test_twoArraysWithDifferentSizesAreNotEqual<T>; });

    utils::bindForScalarTypes(m, "twoArraysWithDifferentShapesButSameSizeAreEqual", []<typename T>() { return &test_twoArraysWithDifferentShapesButSameSizeAreEqual<T>; });

    utils::bindForScalarTypes(m, "arrayOfZerosIsEqualToScalarZero", []<typename T>() { return &test_arrayOfZerosIsEqualToScalarZero<T>; });

    utils::bindForScalarTypes(m, "arrayOfZerosIsDifferentToScalarOne", []<typename T>() { return &test_arrayOfZerosIsDifferentToScalarOne<T>; });

    m.def("rangeIsNeverEqualToScalar", &test_rangeIsNeverEqualToScalar);

    m.def("twoIdenticalArraysContainingStringsAreEqual", &test_twoIdenticalArraysContainingStringsAreEqual);
    m.def("twoIdenticalArraysContainingUnicodeStringsAreEqual", &test_twoIdenticalArraysContainingUnicodeStringsAreEqual);
    m.def("arrayEqualToString", &test_arrayEqualToString);
    m.def("arrayEqualToUnicodeString", &test_arrayEqualToUnicodeString);
    m.def("arrayDifferentToString", &test_arrayDifferentToString);
    m.def("numericalArrayDifferentToString", &test_numericalArrayDifferentToString);
}


# endif