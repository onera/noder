# include <array/factory/c_to_py.hpp>


namespace py = pybind11;


void test_print() {

    Array array = arrayfactory::arrayFromString("test string");
    array.print();
}

void test_ArrayPrintWithNone() {

    Array array;
    std::string arrayPrint = array.getPrintString();
    if (arrayPrint != "None") {
        throw py::value_error("Expected 'None' but got " + arrayPrint);
    }
}

void test_ArrayPrintWithString() {

    Array array = arrayfactory::arrayFromString("test string");
    std::string arrayPrint = array.getPrintString();
    if (arrayPrint != "test string") {
        throw py::value_error("Expected 'test string' but got " + arrayPrint);
    }
}

void test_ArrayPrintWithUnicodeString() {

    Array array = arrayfactory::arrayFromUnicodeString("ρω");
    std::string arrayPrint = array.getPrintString();
    if (arrayPrint != "ρω") {
        throw py::value_error("Expected 'ρω' but got " + arrayPrint);
    }
}

void test_ArrayPrintWithContiguousArray() {

    int carray[2][2] = {{1, 2}, {3, 4}};
    Array array = arrayfactory::toArray2D(&carray[0][0], 2, 2);
    std::string arrayPrint = array.getPrintString();
    if (arrayPrint != "[ 1 2 3 4 ]\n") {
        throw py::value_error("Expected '[ 1 2 3 4 ]' but got " + arrayPrint);
    }
}

void test_ArrayPrintWithNonContiguousArray() {

    int carray[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    Array contiguousArray = arrayfactory::toArray2D(&carray[0][0], 3, 3);
    py::slice rowSlice(1, 3, 1); 
    py::slice colSlice(1, 3, 1);
    py::array nonContiguousPyArray = contiguousArray.getPyArray()[py::make_tuple(rowSlice, colSlice)];
    Array nonContiguousArray = Array(nonContiguousPyArray);

    if (nonContiguousArray.isContiguous()) {
        throw py::value_error("Expected non-contiguous array");
    }

    std::string arrayPrint = nonContiguousArray.getPrintString();
    py::print(contiguousArray);
    py::print(nonContiguousArray);

    if (arrayPrint != "[ 5 6 8 9 ]\n") {
        throw py::value_error("Expected '[ 5 6 8 9 ]' but got " + arrayPrint);
    }
}


void test_ArrayPrintWithLongArrayEven() {

    {
    Array array = arrayfactory::uniformFromStep<int>(0, 100);
    std::string arrayPrint = array.getPrintString(50);
    std::string expected = "[ 0 1 2 3 4 5 6 7 8 9 10...93 94 95 96 97 98 99 ]";
    if (arrayPrint != (expected+"\n")) {
        throw py::value_error("Expected '"+expected+"' but got " + arrayPrint);
    }
    }
}

void test_ArrayPrintWithLongArrayOdd() {
    {
    Array array = arrayfactory::uniformFromStep<int>(0, 101);
    std::string arrayPrint = array.getPrintString(50);
    std::string expected = "[ 0 1 2 3 4 5 6 7 8 9 10...4 95 96 97 98 99 100 ]";
    if (arrayPrint != (expected+"\n")) {
        throw py::value_error("Expected '"+expected+"' but got " + arrayPrint);
    }
    }

}
