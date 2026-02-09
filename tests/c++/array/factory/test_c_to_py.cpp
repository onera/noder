# include "test_c_to_py.hpp"

using namespace arrayfactory;
namespace py = pybind11;

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&test_from_carray_toArray1D<U>), ...);
        (utils::forceSymbol(&test_from_stdarray_toArray1D<U>), ...);
        (utils::forceSymbol(&test_from_vector_toArray1D<U>), ...);
        (utils::forceSymbol(&test_from_carray_toArray2D<U>), ...);
        (utils::forceSymbol(&test_from_stdarray_toArray2D<U>), ...);
        (utils::forceSymbol(&test_from_vector_toArray2D<U>), ...);
        (utils::forceSymbol(&test_from_carray_toArray3D<U>), ...);
        (utils::forceSymbol(&test_from_stdarray_toArray3D<U>), ...);
        (utils::forceSymbol(&test_from_vector_toArray3D<U>), ...);
    }
};



// code

template <typename T>
void test_from_carray_toArray1D( bool copy ) {
    constexpr size_t nbOfItems = 2; // must be the nb of items in carray
    T carray[] = {0, 1};
    Array array = toArray1D<T>(carray,nbOfItems,copy);
    carray[0] = 1; // modification from carray
    size_t lenArray = array.size();
    if (lenArray != nbOfItems) {
        throw py::value_error("expected "+std::to_string(nbOfItems)+" items, but got "+std::to_string(lenArray));
    }

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();
    if (copy and utils::approxEqual(ptr[0],carray[0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],carray[0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_stdarray_toArray1D(bool copy) {
    constexpr size_t nbOfItems = 2; // must be the nb of items in stdarray
    std::array<T, nbOfItems> stdarray = {0, 1};
    Array array = toArray1D<T>(stdarray, copy);
    stdarray[0] = 1; // modification from stdarray
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfItems) {
        throw py::value_error("expected "+std::to_string(nbOfItems)+" items, but got "+std::to_string(lenArray));
    }

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],stdarray[0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],stdarray[0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_vector_toArray1D(bool copy) {
    size_t nbOfItems = 2; // must be the nb of items in vector
    std::vector<T> vector = {0, 1};
    Array array = toArray1D<T>(vector, copy);
    vector[0] = 1; // modification from vector
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfItems) {
        throw py::value_error("expected "+std::to_string(nbOfItems)+" items, but got "+std::to_string(lenArray));
    }

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],vector[0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],vector[0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_carray_toArray2D( bool copy ) {
    constexpr size_t nbOfRows = 2;
    constexpr size_t nbOfCols = 3;

    T carray[nbOfRows][nbOfCols] = {{0, 1, 0}, {1, 0, 1}};
    Array array = toArray2D<T>(&carray[0][0], nbOfRows, nbOfCols, copy);
    carray[0][0] = 1; // modification from carray
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfRows*nbOfCols) throw py::value_error("wrong array size");

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();
    if (copy and utils::approxEqual(ptr[0],carray[0][0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],carray[0][0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_stdarray_toArray2D( bool copy ) {
    constexpr size_t nbOfRows = 2;
    constexpr size_t nbOfCols = 3;

    std::array<std::array<T, nbOfCols>, nbOfRows> stdarray = {{{0, 1, 0}, {1, 0, 1}}};
    Array array = toArray2D<T>(stdarray, copy);
    stdarray[0][0] = 1; // modification from stdarray
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfRows*nbOfCols) throw py::value_error("wrong array size");

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],stdarray[0][0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],stdarray[0][0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_vector_toArray2D( bool copy ) {
    size_t nbOfRows = 2;
    size_t nbOfCols = 3;

    std::vector<std::vector<T>> vector(nbOfRows, std::vector<T>(nbOfCols));
    vector[0] = {0, 1, 0};
    vector[1] = {1, 0, 1};

    Array array = toArray2D<T>(vector, copy);
    vector[0][0] = 1; // modification from vector
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfRows*nbOfCols) throw py::value_error("wrong array size");

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],vector[0][0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],vector[0][0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}

template <typename T>
void test_from_carray_toArray3D( bool copy ) {
    constexpr size_t nbOfRows = 2;
    constexpr size_t nbOfCols = 3;
    constexpr size_t nbOfDepth = 4;

    T carray[nbOfRows][nbOfCols][nbOfDepth] = {
        {{0, 1, 1, 1}, {1, 1, 1, 1}, {0, 0, 0, 1}},
        {{1, 0, 0, 1}, {0, 1, 0, 1}, {1, 0, 1, 0}}
    };
    Array array = toArray3D<T>(&carray[0][0][0], nbOfRows, nbOfCols, nbOfDepth, copy);
    carray[0][0][0] = 1; // modification from carray
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfRows*nbOfCols*nbOfDepth) throw py::value_error("wrong array size");

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],carray[0][0][0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],carray[0][0][0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}

template <typename T>
void test_from_stdarray_toArray3D(bool copy) {
    constexpr size_t nbOfRows = 2;
    constexpr size_t nbOfCols = 3;
    constexpr size_t nbOfDepth = 4;

    std::array<std::array<std::array<T, nbOfDepth>, nbOfCols>, nbOfRows> stdarray = {{
        {{
            {{0, 1, 1, 1}}, {{1, 1, 1, 1}}, {{0, 0, 0, 1}}
        }},
        {{
            {{1, 0, 0, 1}}, {{0, 1, 0, 1}}, {{1, 0, 1, 0}}
        }}
    }};

    Array array = toArray3D<T>(stdarray, copy);
    stdarray[0][0][0] = 1; // Modification from stdarray
    py::print(array);

    size_t lenArray = array.size();
    if (lenArray != nbOfRows * nbOfCols * nbOfDepth) {
        throw py::value_error("wrong array size");
    }

    // Validate memory content
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    std::cout << "ptr[0]: " << static_cast<int>(ptr[0]) << std::endl; // Debug output

    if (copy && utils::approxEqual(ptr[0],stdarray[0][0][0])) {
        throw py::value_error("did not expect modification of array");
    } else if (!copy && !utils::approxEqual(ptr[0],stdarray[0][0][0])) {
        std::cout << "ptr[0]: " << static_cast<int>(ptr[0]) << std::endl;
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template <typename T>
void test_from_vector_toArray3D( bool copy ) {
    size_t nbOfRows = 2;
    size_t nbOfCols = 3;
    size_t nbOfDepth = 4;

    std::vector<std::vector<std::vector<T>>> vector(nbOfRows,
                                                    std::vector<std::vector<T>>(nbOfCols,
                                                                                std::vector<T>(nbOfDepth)));

    vector[0][0] = {0, 1, 1, 1};
    vector[0][1] = {1, 1, 1, 1};
    vector[0][2] = {0, 0, 0, 1};

    vector[1][0] = {1, 0, 0, 1};
    vector[1][1] = {0, 1, 0, 1};
    vector[1][2] = {1, 0, 1, 0};



    Array array = toArray3D<T>(vector, copy);
    vector[0][0][0] = 1; // modification from vector
    py::print(array);
    size_t lenArray = array.size();
    if (lenArray != nbOfRows*nbOfCols*nbOfDepth) throw py::value_error("wrong array size");

    // access the array data
    T* ptr = array.getPointerOfModifiableDataFast<T>();    
    if (copy and utils::approxEqual(ptr[0],vector[0][0][0])) {
        throw py::value_error("did not expected modification of array");
    } else if (!copy and !utils::approxEqual(ptr[0],vector[0][0][0])) {
        throw py::value_error("expected shared data, hence modification of both carray and array");
    }
}


template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();
