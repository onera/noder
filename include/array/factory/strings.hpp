#ifndef ARRAYS_FROM_STRINGS_HPP
#define ARRAYS_FROM_STRINGS_HPP

#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>

# include "array/array.hpp"

namespace arrayfactory {

    Array arrayFromString(const std::string& str);

    Array arrayFromUnicodeString(const std::string& str);

    std::u32string u32stringFromString(const std::string& str);
}

#endif