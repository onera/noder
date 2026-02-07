# ifndef TEST_ARRAY_ASSERTIONS_HPP
# define TEST_ARRAY_ASSERTIONS_HPP

# include <cassert>
# include <array/array.hpp>
# include <array/factory/c_to_py.hpp>
# include <array/factory/vectors.hpp>
# include <array/factory/matrices.hpp>

void test_assertSameSizeAsVector();
void test_assertSameSizeAsArray();
void test_assertSameSizeAsPyArray();

template <typename T>
void test_mustHaveDataOfTypeAndDimensions();

template <typename T>
void test_mustHaveDataOfType();
void test_mustHaveDataOfTypeCatchExpectedError();

void test_mustHaveDataOfDimensions();
void test_mustHaveDataOfDimensionsCatchExpectedError();

template <typename T>
void test_mustHaveValidDataTypeForSettingScalar();
void test_mustHaveValidDataTypeForSettingScalarCatchExpectedError();


# endif