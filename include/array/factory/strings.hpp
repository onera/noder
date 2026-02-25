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

    /** @brief Convert UTF-8 text to a string-backed Array. */
    Array arrayFromString(const std::string& str);

    /** @brief Convert UTF-8 text to a Unicode (`U`) Array. */
    Array arrayFromUnicodeString(const std::string& str);

    /** @brief Convert UTF-8 text to UTF-32 code points. */
    std::u32string u32stringFromString(const std::string& str);
}

#endif
