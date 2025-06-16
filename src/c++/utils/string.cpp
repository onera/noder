# include "utils/string.hpp"

bool utils::stringStartsWith(const std::string& fullString, const std::string& beginning) {
    if (beginning.size() > fullString.size()) return false;
    return fullString.compare(0, beginning.size(), beginning) == 0;
}

bool utils::stringEndsWith(const std::string& fullString, const std::string& ending) {
    if (ending.size() > fullString.size()) return false;
    return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
}

std::string utils::clipStringIfTooLong(const std::string& longString, const size_t& maxChars) {
    
    if (maxChars == 0 ) {
        return longString;
    }
    
    std::string ellipsis = "...";
    size_t clipLength = ellipsis.length();
    if (longString.length() <= maxChars) { return longString; }

    size_t firstHalfLength = (maxChars - clipLength + 1) / 2;
    size_t secondHalfLength = maxChars - firstHalfLength - clipLength;

    std::string clippedString;
    clippedString.reserve(maxChars); // Pre-allocate memory for efficiency

    clippedString.append(longString.substr(0, firstHalfLength));
    clippedString.append(ellipsis);
    clippedString.append(longString.substr(longString.length() - secondHalfLength));

    return clippedString;
}