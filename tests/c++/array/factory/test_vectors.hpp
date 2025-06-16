# ifndef TEST_ARRAY_FACTORY_VECTORS_HPP
# define TEST_ARRAY_FACTORY_VECTORS_HPP

# include <array/factory/vectors.hpp>
# include <utils/comparator.hpp>

template <typename T> [[gnu::used]]
void test_uniformFromCount_positive_step();

template <typename T> [[gnu::used]]
void test_uniformFromCount_negative_step();

void test_uniformFromCount_zero_step();

void test_uniformFromCount_incoherent_step();

template <typename T> [[gnu::used]]
void test_uniformFromCount_endpoint_true();

template <typename T> [[gnu::used]]
void test_uniformFromCount_endpoint_false();

template <typename T> [[gnu::used]]
void test_uniformFromCount_num_zero();

template <typename T> [[gnu::used]]
void test_uniformFromCount_num_one();

template <typename T> [[gnu::used]]
void test_uniformFromCount_floating_point_values();

template <typename T> [[gnu::used]]
void test_uniformFromCount_integer_values();


# endif