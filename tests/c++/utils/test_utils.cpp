# include "test_utils.hpp"

void test_stringStartsWith() {
    if (!utils::stringStartsWith("noder", "no")) {
        throw std::runtime_error("stringStartsWith should return true");
    }
    if (utils::stringStartsWith("noder", "de")) {
        throw std::runtime_error("stringStartsWith should return false");
    }
}

void test_stringEndsWith() {
    if (!utils::stringEndsWith("noder", "er")) {
        throw std::runtime_error("stringEndsWith should return true");
    }
    if (utils::stringEndsWith("noder", "no")) {
        throw std::runtime_error("stringEndsWith should return false");
    }
}

void test_clipStringIfTooLong() {
    const std::string input = "0123456789";
    const std::string clipped = utils::clipStringIfTooLong(input, 6);
    if (clipped.size() > 6) {
        throw std::runtime_error("clipped string should respect max length");
    }
    if (utils::clipStringIfTooLong("abc", 6) != "abc") {
        throw std::runtime_error("short strings should not be clipped");
    }
}

void test_approxEqual() {
    if (!utils::approxEqual(1.0, 1.0)) {
        throw std::runtime_error("approxEqual should match equal doubles");
    }
    if (!utils::approxEqual(3, 3)) {
        throw std::runtime_error("approxEqual should match equal integers");
    }
}
