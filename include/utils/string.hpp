# ifndef UTILS_STRING_HPP
# define UTILS_STRING_HPP

# include <string>
# include <cstdint>

namespace utils {
    bool stringStartsWith(const std::string& fullString, const std::string& beginning);
    bool stringEndsWith(const std::string& fullString, const std::string& ending);
    std::string clipStringIfTooLong(const std::string& longString, const size_t& maxChars);
}

# endif