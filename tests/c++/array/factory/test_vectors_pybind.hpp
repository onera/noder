# ifndef TEST_ARRAY_FACTORY_VECTORS_PYBIND_HPP
# define TEST_ARRAY_FACTORY_VECTORS_PYBIND_HPP

# include "test_vectors.hpp"

void bindTestsOfFactoryOfVectorArrays(py::module_ &m) {

    utils::bindForFloatingAndIntegralTypes(m, "uniformFromCount_positive_step",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_positive_step<T>; }
    );
    utils::bindForFloatingAndIntegralTypes(m, "uniformFromCount_negative_step",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_negative_step<T>; }
    );

    m.def("uniformFromCount_zero_step", &test_uniformFromCount_zero_step);
    m.def("uniformFromCount_incoherent_step", &test_uniformFromCount_incoherent_step);

    utils::bindForFloatingTypes(m, "uniformFromCount_endpoint_true",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_endpoint_true<T>; }
    );
    utils::bindForFloatingTypes(m, "uniformFromCount_endpoint_false",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_endpoint_false<T>; }
    );
    utils::bindForFloatingTypes(m, "uniformFromCount_num_zero",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_num_zero<T>; }
    );
    utils::bindForFloatingTypes(m, "uniformFromCount_num_one",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_num_one<T>; }
    );
    utils::bindForFloatingTypes(m, "uniformFromCount_floating_point_values",
        []<typename T>(utils::TypeTag<T>) { return &test_uniformFromCount_floating_point_values<T>; }
    );
}

# endif