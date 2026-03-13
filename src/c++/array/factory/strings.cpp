#include "array/factory/strings.hpp"

#include <cstring>

namespace arrayfactory {

    Array arrayFromString(const std::string& str) {
        return Array(str);
    }

    Array arrayFromUnicodeString(const std::string& str) {
        std::u32string utf32Str = u32stringFromString(str);
        const size_t stringSize = utf32Str.length();
        const size_t byteCount = stringSize * sizeof(char32_t);

        std::shared_ptr<void> owner;
        if (byteCount != 0) {
            owner = std::shared_ptr<void>(new std::uint8_t[byteCount], [](void* ptr) {
                delete[] static_cast<std::uint8_t*>(ptr);
            });
            std::memcpy(owner.get(), utf32Str.data(), byteCount);
        }

        return Array::unicodeView(owner.get(), byteCount, {1}, {byteCount}, owner);
    }

    std::u32string u32stringFromString(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        return converter.from_bytes(str);
    }

}
