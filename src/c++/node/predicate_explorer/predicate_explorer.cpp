#include "node/predicate_explorer.hpp"

#include "array/array.hpp"
#include "node/node.hpp"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace predicate_explorer {
namespace {

enum class TokenKind {
    End,
    Word,
    Quoted,
    Colon,
    And,
    Or,
    LeftParen,
    RightParen,
    Inward,
    Outward
};

struct Token {
    TokenKind kind;
    std::string text;
    size_t position;
};

std::string parserError(const std::string& expression, size_t position, const std::string& detail) {
    std::ostringstream message;
    message << "invalid predicate at position " << position << ": " << detail
            << " in '" << expression << "'";
    return message.str();
}

std::vector<Token> tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    size_t index = 0;

    while (index < expression.size()) {
        const unsigned char current = static_cast<unsigned char>(expression[index]);
        if (std::isspace(current)) {
            ++index;
            continue;
        }

        const size_t position = index;
        switch (expression[index]) {
            case ':':
                tokens.push_back({TokenKind::Colon, ":", position});
                ++index;
                continue;
            case '&':
                tokens.push_back({TokenKind::And, "&", position});
                ++index;
                continue;
            case '|':
                tokens.push_back({TokenKind::Or, "|", position});
                ++index;
                continue;
            case '(':
                tokens.push_back({TokenKind::LeftParen, "(", position});
                ++index;
                continue;
            case ')':
                tokens.push_back({TokenKind::RightParen, ")", position});
                ++index;
                continue;
            case '/':
                tokens.push_back({TokenKind::Inward, "/", position});
                ++index;
                continue;
            case '\\':
                tokens.push_back({TokenKind::Outward, "\\", position});
                ++index;
                continue;
            case '\'':
            case '"': {
                const char quote = expression[index++];
                std::string value;
                bool closed = false;
                while (index < expression.size()) {
                    const char character = expression[index++];
                    if (character == quote) {
                        if (index < expression.size() && expression[index] == quote) {
                            value.push_back(quote);
                            ++index;
                        } else {
                            closed = true;
                            break;
                        }
                    } else {
                        value.push_back(character);
                    }
                }
                if (!closed) {
                    throw std::invalid_argument(
                        parserError(expression, position, "unterminated quoted value"));
                }
                tokens.push_back({TokenKind::Quoted, std::move(value), position});
                continue;
            }
            default:
                break;
        }

        const size_t begin = index;
        while (index < expression.size()) {
            const char character = expression[index];
            if (std::isspace(static_cast<unsigned char>(character)) ||
                character == ':' || character == '&' || character == '|' ||
                character == '(' || character == ')' || character == '/' ||
                character == '\\') {
                break;
            }
            ++index;
        }
        if (begin == index) {
            throw std::invalid_argument(
                parserError(expression, position, "unexpected character"));
        }
        tokens.push_back({TokenKind::Word, expression.substr(begin, index - begin), begin});
    }

    tokens.push_back({TokenKind::End, std::string(), expression.size()});
    return tokens;
}

enum class AtomKind { Name, Type, Data, Level };
enum class Comparison { Equal, Less, LessEqual, Greater, GreaterEqual };

struct Atom {
    AtomKind kind;
    std::string pattern;
    bool numeric = false;
    Comparison comparison = Comparison::Equal;
    long double numericValue = 0.0L;
    long long levelValue = 0;
};

struct Expression {
    enum class Kind { Atom, And, Or };

    Kind kind = Kind::Atom;
    Atom atom{};
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
};

struct Step {
    enum class Direction { Inward, Outward };

    Direction direction;
    std::shared_ptr<Expression> expression;
};

bool startsPrimary(const Token& token) {
    return token.kind == TokenKind::Word || token.kind == TokenKind::LeftParen;
}

bool parseLongDouble(const std::string& text, long double& value) {
    if (text.empty()) {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    value = std::strtold(text.c_str(), &end);
    return errno != ERANGE && end == text.c_str() + text.size() && std::isfinite(value);
}

bool parseLongLong(const std::string& text, long long& value) {
    if (text.empty()) {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    value = std::strtoll(text.c_str(), &end, 10);
    return errno != ERANGE && end == text.c_str() + text.size();
}

struct ParsedComparison {
    Comparison comparison;
    std::string value;
};

ParsedComparison splitComparison(
    const std::string& value) {

    if (value.rfind(">=", 0) == 0) {
        return {Comparison::GreaterEqual, value.substr(2)};
    }
    if (value.rfind("<=", 0) == 0) {
        return {Comparison::LessEqual, value.substr(2)};
    }
    if (value.rfind(">", 0) == 0) {
        return {Comparison::Greater, value.substr(1)};
    }
    if (value.rfind("<", 0) == 0) {
        return {Comparison::Less, value.substr(1)};
    }
    if (value.rfind("=", 0) == 0) {
        return {Comparison::Equal, value.substr(1)};
    }
    return {Comparison::Equal, value};
}

class Parser {
public:
    explicit Parser(const std::string& expression)
        : _expression(expression), _tokens(tokenize(expression)) {}

    std::vector<Step> parse() {
        std::vector<Step> steps;
        while (peek().kind != TokenKind::End) {
            Step::Direction direction;
            if (accept(TokenKind::Inward)) {
                direction = Step::Direction::Inward;
            } else if (accept(TokenKind::Outward)) {
                direction = Step::Direction::Outward;
            } else {
                fail(peek(), "expected '/' or '\\' traversal direction");
            }

            // ``//predicate`` and ``\\\\predicate`` are accepted as a
            // convenient one-line spelling for a single directional block.
            // This keeps compatibility with the original cgnsviz search
            // sketches while preserving the unambiguous step syntax.
            if ((direction == Step::Direction::Inward && peek().kind == TokenKind::Inward) ||
                (direction == Step::Direction::Outward && peek().kind == TokenKind::Outward)) {
                ++_index;
            }

            if (!startsPrimary(peek())) {
                fail(peek(), "expected a predicate after traversal direction");
            }
            steps.push_back({direction, parseOr()});
        }
        if (steps.empty()) {
            throw std::invalid_argument(parserError(_expression, 0, "predicate is empty"));
        }
        return steps;
    }

private:
    const Token& peek() const {
        return _tokens[_index];
    }

    bool accept(const TokenKind kind) {
        if (peek().kind != kind) {
            return false;
        }
        ++_index;
        return true;
    }

    [[noreturn]] void fail(const Token& token, const std::string& detail) const {
        throw std::invalid_argument(parserError(_expression, token.position, detail));
    }

    std::shared_ptr<Expression> parseOr() {
        auto result = parseAnd();
        while (accept(TokenKind::Or)) {
            if (!startsPrimary(peek())) {
                fail(peek(), "expected a predicate after '|'");
            }
            auto parent = std::make_shared<Expression>();
            parent->kind = Expression::Kind::Or;
            parent->left = result;
            parent->right = parseAnd();
            result = std::move(parent);
        }
        return result;
    }

    std::shared_ptr<Expression> parseAnd() {
        auto result = parsePrimary();
        while (true) {
            const bool explicitAnd = accept(TokenKind::And);
            const bool implicitAnd = !explicitAnd && startsPrimary(peek());
            if (!explicitAnd && !implicitAnd) {
                break;
            }
            if (!startsPrimary(peek())) {
                fail(peek(), "expected a predicate after '&'");
            }
            auto parent = std::make_shared<Expression>();
            parent->kind = Expression::Kind::And;
            parent->left = result;
            parent->right = parsePrimary();
            result = std::move(parent);
        }
        return result;
    }

    std::shared_ptr<Expression> parsePrimary() {
        if (accept(TokenKind::LeftParen)) {
            auto result = parseOr();
            if (!accept(TokenKind::RightParen)) {
                fail(peek(), "expected ')' to close predicate group");
            }
            return result;
        }
        return parseAtom();
    }

    std::shared_ptr<Expression> parseAtom() {
        const Token field = peek();
        if (field.kind != TokenKind::Word) {
            fail(field, "expected predicate field ('n', 't', 'd', or 'l')");
        }
        ++_index;
        if (!accept(TokenKind::Colon)) {
            fail(peek(), "expected ':' after predicate field");
        }
        const Token value = peek();
        if (value.kind != TokenKind::Word && value.kind != TokenKind::Quoted) {
            fail(value, "expected predicate value");
        }
        ++_index;

        Atom atom;
        if (field.text == "n") {
            atom.kind = AtomKind::Name;
            atom.pattern = value.text;
        } else if (field.text == "t") {
            atom.kind = AtomKind::Type;
            atom.pattern = value.text;
        } else if (field.text == "d") {
            atom.kind = AtomKind::Data;
            atom.pattern = value.text;
            const bool quoted = value.kind == TokenKind::Quoted;
            const ParsedComparison comparison = splitComparison(value.text);
            long double numericValue = 0.0L;
            const bool hasComparisonPrefix = value.text.rfind("<", 0) == 0 ||
                value.text.rfind(">", 0) == 0 || value.text.rfind("=", 0) == 0;
            if (!quoted && parseLongDouble(comparison.value, numericValue)) {
                atom.numeric = true;
                atom.comparison = comparison.comparison;
                atom.numericValue = numericValue;
            } else if (!quoted && hasComparisonPrefix) {
                fail(value, "numeric data comparison has an invalid number");
            }
        } else if (field.text == "l") {
            atom.kind = AtomKind::Level;
            const ParsedComparison comparison = splitComparison(value.text);
            long long level = 0;
            if (value.kind == TokenKind::Quoted || !parseLongLong(comparison.value, level) || level < 0) {
                fail(value, "level must be a non-negative integer comparison");
            }
            atom.comparison = comparison.comparison;
            atom.levelValue = level;
        } else {
            fail(field, "unknown predicate field '" + field.text + "'");
        }

        auto result = std::make_shared<Expression>();
        result->atom = std::move(atom);
        return result;
    }

    const std::string& _expression;
    std::vector<Token> _tokens;
    size_t _index = 0;
};

bool globMatch(const std::string& pattern, const std::string& value) {
    size_t patternIndex = 0;
    size_t valueIndex = 0;
    size_t starIndex = std::string::npos;
    size_t backtrackValueIndex = 0;

    while (valueIndex < value.size()) {
        if (patternIndex < pattern.size() &&
            (pattern[patternIndex] == '?' || pattern[patternIndex] == value[valueIndex])) {
            ++patternIndex;
            ++valueIndex;
        } else if (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
            starIndex = patternIndex++;
            backtrackValueIndex = valueIndex;
        } else if (starIndex != std::string::npos) {
            patternIndex = starIndex + 1;
            valueIndex = ++backtrackValueIndex;
        } else {
            return false;
        }
    }

    while (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
        ++patternIndex;
    }
    return patternIndex == pattern.size();
}

bool compare(long double left, Comparison comparison, long double right) {
    switch (comparison) {
        case Comparison::Equal: return left == right;
        case Comparison::Less: return left < right;
        case Comparison::LessEqual: return left <= right;
        case Comparison::Greater: return left > right;
        case Comparison::GreaterEqual: return left >= right;
    }
    return false;
}

bool compare(long long left, Comparison comparison, long long right) {
    switch (comparison) {
        case Comparison::Equal: return left == right;
        case Comparison::Less: return left < right;
        case Comparison::LessEqual: return left <= right;
        case Comparison::Greater: return left > right;
        case Comparison::GreaterEqual: return left >= right;
    }
    return false;
}

bool numericValue(const Data& data, long double& value) {
    const auto* array = dynamic_cast<const Array*>(&data);
    if (array == nullptr || !array->isScalar() || array->size() == 0) {
        return false;
    }

    switch (array->typeId()) {
        case ArrayTypeId::Bool: value = array->getItemAtIndex<bool>(0) ? 1.0L : 0.0L; return true;
        case ArrayTypeId::Int8: value = array->getItemAtIndex<int8_t>(0); return true;
        case ArrayTypeId::Int16: value = array->getItemAtIndex<int16_t>(0); return true;
        case ArrayTypeId::Int32: value = array->getItemAtIndex<int32_t>(0); return true;
        case ArrayTypeId::Int64: value = array->getItemAtIndex<int64_t>(0); return true;
        case ArrayTypeId::UInt8: value = array->getItemAtIndex<uint8_t>(0); return true;
        case ArrayTypeId::UInt16: value = array->getItemAtIndex<uint16_t>(0); return true;
        case ArrayTypeId::UInt32: value = array->getItemAtIndex<uint32_t>(0); return true;
        case ArrayTypeId::UInt64: value = array->getItemAtIndex<uint64_t>(0); return true;
        case ArrayTypeId::Float32: value = array->getItemAtIndex<float>(0); return true;
        case ArrayTypeId::Float64: value = array->getItemAtIndex<double>(0); return true;
        case ArrayTypeId::None:
        case ArrayTypeId::Bytes:
        case ArrayTypeId::Unicode:
            return false;
    }
    return false;
}

bool matchesAtom(const Atom& atom, const Node& node, const size_t level) {
    switch (atom.kind) {
        case AtomKind::Name:
            return globMatch(atom.pattern, node.name());
        case AtomKind::Type:
            return globMatch(atom.pattern, node.type());
        case AtomKind::Level:
            if (level > static_cast<size_t>(std::numeric_limits<long long>::max())) {
                return atom.comparison == Comparison::Less ||
                    atom.comparison == Comparison::LessEqual;
            }
            return compare(static_cast<long long>(level), atom.comparison, atom.levelValue);
        case AtomKind::Data:
            break;
    }

    if (!node.hasData()) {
        return false;
    }

    if (!atom.numeric) {
        const Data& data = node.data();
        return data.hasString() && globMatch(atom.pattern, data.extractString());
    }

    const std::optional<bool> scalar = node.dataIsScalar();
    if (scalar.has_value() && !scalar.value()) {
        return false;
    }
    const Data& data = node.data();
    long double value = 0.0L;
    return numericValue(data, value) && compare(value, atom.comparison, atom.numericValue);
}

bool matchesExpression(const std::shared_ptr<Expression>& expression, const Node& node, size_t level) {
    if (expression->kind == Expression::Kind::Atom) {
        return matchesAtom(expression->atom, node, level);
    }
    if (expression->kind == Expression::Kind::And) {
        return matchesExpression(expression->left, node, level) &&
            matchesExpression(expression->right, node, level);
    }
    return matchesExpression(expression->left, node, level) ||
        matchesExpression(expression->right, node, level);
}

void appendUnique(
    const std::shared_ptr<Node>& node,
    std::vector<std::shared_ptr<Node>>& matches,
    std::unordered_set<const Node*>& seen) {
    if (node && seen.insert(node.get()).second) {
        matches.push_back(node);
    }
}

void visitInward(
    const std::shared_ptr<Node>& parent,
    const std::shared_ptr<Expression>& expression,
    size_t level,
    std::vector<std::shared_ptr<Node>>& matches,
    std::unordered_set<const Node*>& seen) {

    for (const auto& child : parent->children()) {
        if (!child) {
            continue;
        }
        if (matchesExpression(expression, *child, level)) {
            appendUnique(child, matches, seen);
        }
        visitInward(child, expression, level + 1, matches, seen);
    }
}

void visitOutward(
    const std::shared_ptr<Node>& start,
    const std::shared_ptr<Expression>& expression,
    std::vector<std::shared_ptr<Node>>& matches,
    std::unordered_set<const Node*>& seen) {

    std::shared_ptr<Node> current = start->parent().lock();
    size_t level = 1;
    while (current) {
        if (matchesExpression(expression, *current, level)) {
            appendUnique(current, matches, seen);
        }
        current = current->parent().lock();
        ++level;
    }
}

} // namespace

std::vector<std::shared_ptr<Node>> allByPredicate(
    Node& start,
    const std::string& expression) {

    const std::vector<Step> steps = Parser(expression).parse();
    std::shared_ptr<Node> startPointer = start.selfPtr();
    if (!startPointer) {
        throw std::invalid_argument(
            "predicate explorer requires the starting Node to be managed by shared_ptr");
    }

    std::vector<std::shared_ptr<Node>> current{std::move(startPointer)};
    for (const Step& step : steps) {
        if (current.empty()) {
            return {};
        }

        std::vector<std::shared_ptr<Node>> next;
        std::unordered_set<const Node*> seen;
        for (const auto& node : current) {
            if (step.direction == Step::Direction::Inward) {
                visitInward(node, step.expression, 1, next, seen);
            } else {
                visitOutward(node, step.expression, next, seen);
            }
        }
        current = std::move(next);
    }
    return current;
}

std::shared_ptr<Node> byPredicate(Node& start, const std::string& expression) {
    const auto matches = allByPredicate(start, expression);
    return matches.empty() ? nullptr : matches.front();
}

} // namespace predicate_explorer
