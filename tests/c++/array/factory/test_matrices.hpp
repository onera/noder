# ifndef TEST_ARRAY_FACTORY_MATRICES_HPP
# define TEST_ARRAY_FACTORY_MATRICES_HPP

# include <array/factory/matrices.hpp>
# include <utils/template_instantiator.hpp>
# include <utils/comparator.hpp>


template <typename T> [[gnu::used]]
void test_zeros_c_order();

template <typename T> [[gnu::used]]
void test_zeros_f_order();

template <typename T> [[gnu::used]]
void test_ones_c_order();

template <typename T> [[gnu::used]]
void test_ones_f_order();

template <typename T> [[gnu::used]]
void test_full_c_order();

template <typename T> [[gnu::used]]
void test_full_f_order();

template <typename T> [[gnu::used]]
void test_empty_c_order();

template <typename T> [[gnu::used]]
void test_empty_f_order();


# endif // TEST_ARRAY_FACTORY_MATRICES_HPP