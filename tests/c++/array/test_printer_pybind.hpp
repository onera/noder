# ifndef TEST_ARRAY_PRINTER_PYBIND_HPP
# define TEST_ARRAY_PRINTER_PYBIND_HPP

# include "test_printer.hpp"

void bindTestsOfArrayPrinter(py::module_ &m) {

    m.def("print", &test_print);
    m.def("ArrayPrintWithNone", &test_ArrayPrintWithNone);
    m.def("ArrayPrintWithString", &test_ArrayPrintWithString);
    m.def("ArrayPrintWithUnicodeString", &test_ArrayPrintWithUnicodeString);
    m.def("ArrayPrintWithContiguousArray", &test_ArrayPrintWithContiguousArray);
    m.def("ArrayPrintWithNonContiguousArray", &test_ArrayPrintWithNonContiguousArray);
    m.def("ArrayPrintWithLongArrayEven", &test_ArrayPrintWithLongArrayEven);
    m.def("ArrayPrintWithLongArrayOdd", &test_ArrayPrintWithLongArrayOdd);

}

# endif