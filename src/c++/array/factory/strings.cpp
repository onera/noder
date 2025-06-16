# include "array/factory/strings.hpp"


namespace arrayfactory {

    namespace py = pybind11;

    Array arrayFromString(const std::string& str) {

        size_t stringSize = str.length();  

        std::vector<char> buffer(stringSize);
        std::memcpy(buffer.data(), str.c_str(), stringSize);

        py::array pyArray(py::dtype("S"+std::to_string(stringSize)),
                          {1}, // shape: we do not allow multi-dimensional string arrays
                          buffer.data());

        Array array(pyArray);

        return array;
    }

    Array arrayFromUnicodeString(const std::string& str) {

        std::u32string utf32_str = u32stringFromString(str);

        size_t stringSize = utf32_str.length();
        auto data_ptr = reinterpret_cast<const char32_t*>(utf32_str.data());

        py::array pyArray(
            py::dtype("U" + std::to_string(stringSize)), 
            {1},                             // shape
            {sizeof(char32_t) * stringSize}, // Strides
            data_ptr                         
        );

        Array array(pyArray);

        return array;

    }

    std::u32string u32stringFromString(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        return converter.from_bytes(str);
    }

} 