#ifndef TEST_C_TO_PY_H
#define TEST_C_TO_PY_H

#include <array/factory/c_to_py.hpp>
#include <utils/comparator.hpp>

template <typename T> [[gnu::used]]
void test_from_carray_toArray1D(bool copy=false);

template <typename T> [[gnu::used]]
void test_from_stdarray_toArray1D(bool copy=false);

template <typename T> [[gnu::used]]
void test_from_vector_toArray1D(bool copy=false);

template <typename T> [[gnu::used]]
void test_from_carray_toArray2D( bool copy=false);

template <typename T> [[gnu::used]]
void test_from_stdarray_toArray2D( bool copy=false);

template <typename T> [[gnu::used]]
void test_from_vector_toArray2D( bool copy=false);

template <typename T> [[gnu::used]]
void test_from_carray_toArray3D( bool copy=false);

template <typename T> [[gnu::used]]
void test_from_stdarray_toArray3D( bool copy=false);

template <typename T> [[gnu::used]]
void test_from_vector_toArray3D( bool copy=false);

#endif // TEST_C_TO_PY_H