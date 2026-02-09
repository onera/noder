# ifndef TEST_ARRAY_ASSERTIONS_PYBIND_HPP
# define TEST_ARRAY_ASSERTIONS_PYBIND_HPP

# include "test_assertions.hpp"

void bindTestsOfArrayAssertions(py::module_ &m) {
    
    m.def("assertSameSizeAsVector", &test_assertSameSizeAsVector);
    m.def("assertSameSizeAsArray", &test_assertSameSizeAsArray);
    m.def("assertSameSizeAsPyArray", &test_assertSameSizeAsPyArray);

    utils::bindForScalarTypes(m, "mustHaveDataOfTypeAndDimensions", 
        []<typename T>(utils::TypeTag<T>) { return &test_mustHaveDataOfTypeAndDimensions<T>; }
    );
    utils::bindForScalarTypes(m, "mustHaveDataOfType", 
        []<typename T>(utils::TypeTag<T>) { return &test_mustHaveDataOfType<T>; }
    );

    m.def("mustHaveDataOfTypeCatchExpectedError", &test_mustHaveDataOfTypeCatchExpectedError);

    m.def("mustHaveDataOfDimensions", &test_mustHaveDataOfDimensions);
    m.def("mustHaveDataOfDimensionsCatchExpectedError", &test_mustHaveDataOfDimensionsCatchExpectedError);

    utils::bindForScalarTypes(m, "mustHaveValidDataTypeForSettingScalar", 
        []<typename T>(utils::TypeTag<T>) { return &test_mustHaveValidDataTypeForSettingScalar<T>; }
    );

    m.def("mustHaveValidDataTypeForSettingScalarCatchExpectedError", &test_mustHaveValidDataTypeForSettingScalarCatchExpectedError);
}

# endif