# ifndef TEST_ARRAY_DATA_ACCESSORS_HPP
# define TEST_ARRAY_DATA_ACCESSORS_HPP

# include <array/array.hpp>
# include <array/factory/c_to_py.hpp>
# include <array/factory/vectors.hpp>

template <typename T> [[gnu::used]]
void test_scalarSlicingProducesScalar();

template <typename T> [[gnu::used]]
void test_getItemAsArrayAtIndex();

template <typename T> [[gnu::used]]
void test_scalarSlicingDoesNotMakeCopy();

template <typename T> [[gnu::used]]
void test_getItemAtIndex();

template <typename T> [[gnu::used]]
void test_getPointerOfDataSafely();

template <typename T> [[gnu::used]]
void test_getPointerOfModifiableDataFast();

template <typename T> [[gnu::used]]
void test_getPointerOfReadOnlyDataFast();

template <typename T> [[gnu::used]]
void test_getAccessorOfReadOnlyData();

template <typename T> [[gnu::used]]
void test_getAccessorOfModifiableData();

void test_getFlatIndexOfArrayInStyleC();

void test_getFlatIndexOfArrayInStyleFortran();

void test_extractStringAscii();

void test_extractStringUnicode();

# endif