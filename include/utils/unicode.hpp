#ifndef NODER_UTILS_UNICODE_HPP
#define NODER_UTILS_UNICODE_HPP

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace noder::unicode {

inline std::u32string from_utf8(const std::string_view input) {
    std::u32string output;
    output.reserve(input.size());

    for (std::size_t i = 0; i < input.size();) {
        const auto first = static_cast<std::uint8_t>(input[i]);
        std::uint32_t code_point = 0;
        std::size_t sequence_length = 0;
        std::uint32_t minimum = 0;

        if (first <= 0x7FU) {
            code_point = first;
            sequence_length = 1;
        } else if (first >= 0xC2U && first <= 0xDFU) {
            code_point = first & 0x1FU;
            sequence_length = 2;
            minimum = 0x80U;
        } else if (first >= 0xE0U && first <= 0xEFU) {
            code_point = first & 0x0FU;
            sequence_length = 3;
            minimum = 0x800U;
        } else if (first >= 0xF0U && first <= 0xF4U) {
            code_point = first & 0x07U;
            sequence_length = 4;
            minimum = 0x10000U;
        } else {
            throw std::range_error("invalid UTF-8 sequence");
        }

        if (i + sequence_length > input.size()) {
            throw std::range_error("truncated UTF-8 sequence");
        }
        for (std::size_t j = 1; j < sequence_length; ++j) {
            const auto continuation = static_cast<std::uint8_t>(input[i + j]);
            if ((continuation & 0xC0U) != 0x80U) {
                throw std::range_error("invalid UTF-8 continuation byte");
            }
            code_point = (code_point << 6U) | (continuation & 0x3FU);
        }
        if (code_point < minimum || code_point > 0x10FFFFU
            || (code_point >= 0xD800U && code_point <= 0xDFFFU)) {
            throw std::range_error("invalid UTF-8 code point");
        }

        output.push_back(static_cast<char32_t>(code_point));
        i += sequence_length;
    }
    return output;
}

inline std::string to_utf8(const std::u32string_view input) {
    std::string output;
    output.reserve(input.size());

    for (const char32_t character : input) {
        const auto code_point = static_cast<std::uint32_t>(character);
        if (code_point > 0x10FFFFU || (code_point >= 0xD800U && code_point <= 0xDFFFU)) {
            throw std::range_error("invalid UTF-32 code point");
        }
        if (code_point <= 0x7FU) {
            output.push_back(static_cast<char>(code_point));
        } else if (code_point <= 0x7FFU) {
            output.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
            output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        } else if (code_point <= 0xFFFFU) {
            output.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
            output.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        } else {
            output.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
            output.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        }
    }
    return output;
}

}  // namespace noder::unicode

#endif
