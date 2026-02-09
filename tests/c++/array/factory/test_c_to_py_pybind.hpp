# ifndef TEST_ARRAY_FACTORY_C_TO_PY_PYBIND_HPP
# define TEST_ARRAY_FACTORY_C_TO_PY_PYBIND_HPP

# include "test_c_to_py.hpp"

void bindTestsOfFactoryOfArraysFromCToPy(py::module_ &m) {

    utils::bindForScalarTypes(m, "from_carray_toArray1D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_carray_toArray1D<T>; }
    );

    utils::bindForScalarTypes(m, "from_stdarray_toArray1D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_stdarray_toArray1D<T>; }
    );

    utils::bindForScalarTypes(m, "from_vector_toArray1D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_vector_toArray1D<T>; }
    );


    utils::bindForScalarTypes(m, "from_carray_toArray2D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_carray_toArray2D<T>; }
    );

    utils::bindForScalarTypes(m, "from_stdarray_toArray2D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_stdarray_toArray2D<T>; }
    );

    utils::bindForScalarTypes(m, "from_vector_toArray2D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_vector_toArray2D<T>; }
    );


    utils::bindForScalarTypes(m, "from_carray_toArray3D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_carray_toArray3D<T>; }
    );

    utils::bindForScalarTypes(m, "from_stdarray_toArray3D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_stdarray_toArray3D<T>; }
    );

    utils::bindForScalarTypes(m, "from_vector_toArray3D",
        []<typename T>(utils::TypeTag<T>) { return &test_from_vector_toArray3D<T>; }
    );

}

# endif 