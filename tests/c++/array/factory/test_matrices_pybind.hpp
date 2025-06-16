# ifndef TEST_ARRAY_FACTORY_MATRICES_PYBIND_HPP
# define TEST_ARRAY_FACTORY_MATRICES_PYBIND_HPP

# include "test_matrices.hpp"

void bindTestsOfFactoryOfMatricesArrays(py::module_ &m) {

    utils::bindForFloatingAndIntegralTypes(m, "zeros_c_order", []<typename T>() { return &test_zeros_c_order<T>; });
    utils::bindForFloatingAndIntegralTypes(m, "zeros_f_order", []<typename T>() { return &test_zeros_f_order<T>; });

    utils::bindForFloatingAndIntegralTypes(m, "ones_c_order", []<typename T>() { return &test_ones_c_order<T>; });
    utils::bindForFloatingAndIntegralTypes(m, "ones_f_order", []<typename T>() { return &test_ones_f_order<T>; });

    utils::bindForFloatingAndIntegralTypes(m, "full_c_order", []<typename T>() { return &test_full_c_order<T>; });
    utils::bindForFloatingAndIntegralTypes(m, "full_f_order", []<typename T>() { return &test_full_f_order<T>; });

    utils::bindForFloatingAndIntegralTypes(m, "empty_c_order", []<typename T>() { return &test_empty_c_order<T>; });
    utils::bindForFloatingAndIntegralTypes(m, "empty_f_order", []<typename T>() { return &test_empty_f_order<T>; });
}

# endif