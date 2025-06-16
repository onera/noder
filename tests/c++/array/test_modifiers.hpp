# ifndef TEST_ARRAY_MODIFIERS_HPP
# define TEST_ARRAY_MODIFIERS_HPP

# include <array/array.hpp>
# include <array/factory/vectors.hpp>
# include <array/factory/matrices.hpp>
# include <array/factory/c_to_py.hpp>

template <typename T> [[gnu::used]]
void test_setArrayToScalar();
void test_catchErrorWhenAssigningWrongScalarType();

void test_catchErrorWhenAssigningScalarToStringArray();

template <typename T> [[gnu::used]]
void test_setFromArrayConsideringAllTypes();
void test_setFromArrayToRange();

template <typename T> [[gnu::used]]
void test_addScalarConsideringAllTypes();

void test_addScalarToRange();
void test_substractScalarToRange();

template <typename T> [[gnu::used]]
void test_multiplyScalarConsideringAllTypes();

void test_multiplyScalarToRange();
void test_divideScalarToRangeOfIntegers();
void test_divideScalarToRangeOfFloats();

template <typename T> [[gnu::used]]
void test_addFromArrayConsideringAllTypes();
void test_addFromArrayToRange();

template <typename T> [[gnu::used]]
void test_substractFromArrayConsideringAllTypes();
void test_substractFromArrayToRange();

template <typename T> [[gnu::used]]
void test_multiplyFromArrayConsideringAllTypes();
void test_multiplyFromArrayToRange();


template <typename T> [[gnu::used]]
void test_divideFromArrayConsideringAllTypes();
void test_divideFromArrayToRange();

# endif