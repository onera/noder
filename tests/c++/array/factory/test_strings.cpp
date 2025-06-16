# include "test_strings.hpp"

using namespace arrayfactory;

void test_arrayFromString() {
    std::string testString = "test string";
    Array result = arrayFromString(testString);
    if (!result.hasString()) {
        throw std::runtime_error("arrayFromString failed: wrong dtype");
    }

    if (result.size() != 1) {
        throw std::runtime_error("arrayFromString failed: wrong size");
    }

    std::string resultString = result.extractString();
    if (resultString != testString) {
        throw std::runtime_error("arrayFromString failed: incorrect string");
    }
}


void test_arrayFromUnicodeString() {
    std::string testString = "ρω";
    Array result = arrayFromUnicodeString(testString);
    py::dtype dtype = result.getPyArray().dtype();
    if (dtype.kind() != 'U') {
        throw py::value_error("arrayFromUnicodeString failed: wrong dtype");
    }

    if (result.size() != 1) {
        throw py::value_error("arrayFromUnicodeString failed: wrong size");
    }

    std::string decodedString = result.extractString();
    if (decodedString != testString) {
        throw py::value_error("arrayFromUnicodeString failed: incorrect string");
    }
}
