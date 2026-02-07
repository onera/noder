# ifndef TEST_ARRAY_DATA_ACCESSORS_HPP
# define TEST_ARRAY_DATA_ACCESSORS_HPP

# include <array/array.hpp>
# include <array/factory/c_to_py.hpp>
# include <array/factory/vectors.hpp>

template <typename T>
void test_scalarSlicingProducesScalar();

template <typename T>
void test_getItemAsArrayAtIndex();

template <typename T>
void test_scalarSlicingDoesNotMakeCopy();

template <typename T>
void test_getItemAtIndex();

template <typename T>
void test_getPointerOfDataSafely();

template <typename T>
void test_getPointerOfModifiableDataFast();

template <typename T>
void test_getPointerOfReadOnlyDataFast();

template <typename T>
void test_getAccessorOfReadOnlyData();

template <typename T>
void test_getAccessorOfModifiableData();

void test_getFlatIndexOfArrayInStyleC();

void test_getFlatIndexOfArrayInStyleFortran();

void test_extractStringAscii();

void test_extractStringUnicode();

# endif