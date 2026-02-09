# ifndef TEST_ARRAY_DATA_ACCESSORS_PYBIND_HPP
# define TEST_ARRAY_DATA_ACCESSORS_PYBIND_HPP

# include "test_array_data_accessors.hpp"

void bindTestsOfArrayDataAccessors(py::module_ &m) {

    utils::bindForScalarTypes(m, "scalarSlicingProducesScalar",
        []<typename T>(utils::TypeTag<T>) { return &test_scalarSlicingProducesScalar<T>; }
    );

    utils::bindForScalarTypes(m, "scalarSlicingDoesNotMakeCopy",
        []<typename T>(utils::TypeTag<T>) { return &test_scalarSlicingDoesNotMakeCopy<T>; }
    );

    utils::bindForScalarTypes(m, "getItemAtIndex", 
        []<typename T>(utils::TypeTag<T>) { return &test_getItemAtIndex<T>; }
    );

    utils::bindForScalarTypes(m, "getPointerOfDataSafely", 
        []<typename T>(utils::TypeTag<T>) { return &test_getPointerOfDataSafely<T>; }
    );

    utils::bindForScalarTypes(m, "getPointerOfModifiableDataFast", 
        []<typename T>(utils::TypeTag<T>) { return &test_getPointerOfModifiableDataFast<T>; }
    );

    utils::bindForScalarTypes(m, "getPointerOfReadOnlyDataFast", 
        []<typename T>(utils::TypeTag<T>) { return &test_getPointerOfReadOnlyDataFast<T>; }
    );

    utils::bindForFloatingAndIntegralTypes(m, "getAccessorOfReadOnlyData", 
        []<typename T>(utils::TypeTag<T>) { return &test_getAccessorOfReadOnlyData<T>; }
    );

    utils::bindForFloatingAndIntegralTypes(m, "getAccessorOfModifiableData", 
        []<typename T>(utils::TypeTag<T>) { return &test_getAccessorOfModifiableData<T>; }
    );
    
    m.def("getFlatIndexOfArrayInStyleC", &test_getFlatIndexOfArrayInStyleC);
    m.def("getFlatIndexOfArrayInStyleFortran", &test_getFlatIndexOfArrayInStyleFortran);

    m.def("extractStringAscii", &test_extractStringAscii);
    m.def("extractStringUnicode", &test_extractStringUnicode);
}

# endif