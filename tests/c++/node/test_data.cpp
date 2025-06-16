# include "test_data.hpp"
# include <array/array.hpp>

namespace testdata {

void test_hasString() {
    Node a;
    if (a.data().hasString()) throw py::value_error("expected no string");
}


void test_isNone() {
    Node a;
    if (!a.data().isNone()) throw py::value_error("expected data none");
}

void test_isScalar() {
    Node a;
    if (a.data().isScalar()) throw py::value_error("expected no scalar");
}

void test_expectedString() {
    Node a;
    Array array(std::string("test string"));
    a.setData(array);
    if (!a.data().hasString()) throw py::value_error("expected string");
}

}
