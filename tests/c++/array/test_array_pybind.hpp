# ifndef TEST_ARRAY_PYBIND_HPP
# define TEST_ARRAY_PYBIND_HPP

# include <pybind11/pybind11.h>

# include "test_array.hpp"
# include "factory/test_factory_pybind.hpp"
# include "test_printer_pybind.hpp"
# include "test_array_data_accessors_pybind.hpp"
# include "test_comparisons_pybind.hpp"
# include "test_modifiers_pybind.hpp"
# include "test_assertions_pybind.hpp"

void bindTestsOfArray(py::module_ &m) {

    py::module_ sm = m.def_submodule("array");


    sm.def("constructorEmpty", &test_constructorEmpty);
    sm.def("constructorPyArray", &test_constructorPyArray);
    sm.def("constructorString", &test_constructorString);
    sm.def("constructorAnotherArray", &test_constructorAnotherArray);

    sm.def("getArrayProperties", &test_getArrayProperties);

    sm.def("sharingData", &test_sharingData);

    sm.def("arrayWithStringHasStringTrue", &test_arrayWithStringHasStringTrue);
    sm.def("arrayWithUnicodeStringHasStringTrue", &test_arrayWithUnicodeStringHasStringTrue);
    sm.def("arrayWithNumbersHasStringFalse", &test_arrayWithNumbersHasStringFalse);

    sm.def("isNone", &test_isNone);
    sm.def("arrayWithNumbersIsNotNone", &test_arrayWithNumbersIsNotNone);
    sm.def("arrayWithStringIsNotNone", &test_arrayWithStringIsNotNone);
    sm.def("arrayWithNumberOfSizeZeroIsNone", &test_arrayWithNumberOfSizeZeroIsNone);

    utils::bindForScalarTypes(sm, "isScalar", 
        []<typename T>(utils::TypeTag<T>) { return &test_isScalar<T>; }
    );

    utils::bindForScalarTypes(sm, "contiguity", 
        []<typename T>(utils::TypeTag<T>) { return &test_contiguity<T>; }
    );

    utils::bindForScalarTypes(sm, "hasDataOfType", 
        []<typename T>(utils::TypeTag<T>) { return &test_hasDataOfType<T>; }
    );

    sm.def("doNotHaveDataOfType", &test_doNotHaveDataOfType);


    bindTestsOfFactoryOfArrays(sm);
    bindTestsOfArrayPrinter(sm);
    bindTestsOfArrayDataAccessors(sm);    
    bindTestsOfArrayComparisons(sm);
    bindTestsOfArrayModifiers(sm);
    bindTestsOfArrayAssertions(sm);
}

# endif