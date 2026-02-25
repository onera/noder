#ifndef TEST_ARRAY_HPP
#define TEST_ARRAY_HPP

# include <cassert>
# include <array/array.hpp>
# include <array/factory/c_to_py.hpp>
# include <array/factory/vectors.hpp>
# include <utils/template_instantiator.hpp>

void test_constructorEmpty();
void test_constructorPyArray();
void test_constructorString();
void test_constructorAnotherArray();

void test_getArrayProperties();
void test_getFlatIndex();
void test_getPyArray();
void test_extractString();
void test_getPrintStringAndInfo();

void test_sharingData();

void test_arrayWithStringHasStringTrue();
void test_arrayWithUnicodeStringHasStringTrue();
void test_arrayWithNumbersHasStringFalse();

void test_isNone();
void test_arrayWithNumbersIsNotNone();
void test_arrayWithStringIsNotNone();
void test_arrayWithNumberOfSizeZeroIsNone();

template <typename T>
void test_isScalar();

template <typename T>
void test_contiguity();

template <typename T>
void test_hasDataOfType();

void test_doNotHaveDataOfType();

void test_getPointerOfDataSafely();
void test_getPointerOfModifiableDataFast();
void test_getPointerOfReadOnlyDataFast();


#endif
