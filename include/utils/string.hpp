# ifndef UTILS_STRING_HPP
# define UTILS_STRING_HPP

# include <string>
# include <cstdint>

namespace utils {
    /** @brief True when @p fullString begins with @p beginning. */
    bool stringStartsWith(const std::string& fullString, const std::string& beginning);
    /** @brief True when @p fullString ends with @p ending. */
    bool stringEndsWith(const std::string& fullString, const std::string& ending);
    /** @brief Clip long text to @p maxChars, appending ellipsis when needed. */
    std::string clipStringIfTooLong(const std::string& longString, const size_t& maxChars);
}

# endif
