# ifndef TEST_ARRAY_MODIFIERS_PYBIND_HPP
# define TEST_ARRAY_MODIFIERS_PYBIND_HPP

# include "test_modifiers.hpp"


void bindTestsOfArrayModifiers(py::module_ &m) {

    // just a reminder on how to explicitly set the types:
    //
    // utils::bindForSpecifiedTypeList(m, "setArrayToScalar", 
    //         utils::TypeList<bool, float, double,
    //         int8_t,  int16_t,  int32_t,  int64_t,
    //         uint8_t, uint16_t, uint32_t, uint64_t>{}, 
    //     []<typename T>() { return &test_setArrayToScalar<T>; });
    //
    // but for convenience we use:
    utils::bindForScalarTypes(m, "setArrayToScalar", []<typename T>() { return &test_setArrayToScalar<T>;});

    utils::bindForScalarTypes(m, "setFromArrayConsideringAllTypes", []<typename T>() { return &test_setFromArrayConsideringAllTypes<T>;});

    m.def("setFromArrayToRange", &test_setFromArrayToRange);   
    
    m.def("catchErrorWhenAssigningWrongScalarType", &test_catchErrorWhenAssigningWrongScalarType);
    m.def("catchErrorWhenAssigningScalarToStringArray", &test_catchErrorWhenAssigningScalarToStringArray);

    utils::bindForFloatingAndIntegralTypes(m, "addScalarConsideringAllTypes", []<typename T>() { return &test_addScalarConsideringAllTypes<T>;});

    m.def("addScalarToRange", &test_addScalarToRange);   
    m.def("substractScalarToRange", &test_substractScalarToRange);   

    utils::bindForFloatingAndIntegralTypes(m, "multiplyScalarConsideringAllTypes", []<typename T>() { return &test_multiplyScalarConsideringAllTypes<T>;});

    m.def("multiplyScalarToRange", &test_multiplyScalarToRange);   
    m.def("divideScalarToRangeOfIntegers", &test_divideScalarToRangeOfIntegers);
    m.def("divideScalarToRangeOfFloats", &test_divideScalarToRangeOfFloats);

    utils::bindForFloatingAndIntegralTypes(m, "addFromArrayConsideringAllTypes", []<typename T>() { return &test_addFromArrayConsideringAllTypes<T>;});

    m.def("addFromArrayToRange", &test_addFromArrayToRange);   

    utils::bindForFloatingAndIntegralTypes(m, "substractFromArrayConsideringAllTypes", []<typename T>() { return &test_substractFromArrayConsideringAllTypes<T>;});

    m.def("substractFromArrayToRange", &test_substractFromArrayToRange);   

    utils::bindForFloatingAndIntegralTypes(m, "multiplyFromArrayConsideringAllTypes", []<typename T>() { return &test_multiplyFromArrayConsideringAllTypes<T>;});

    m.def("multiplyFromArrayToRange", &test_multiplyFromArrayToRange);   

    utils::bindForFloatingAndIntegralTypes(m, "divideFromArrayConsideringAllTypes", []<typename T>() { return &test_divideFromArrayConsideringAllTypes<T>;});

    m.def("divideFromArrayToRange", &test_divideFromArrayToRange);
}

# endif