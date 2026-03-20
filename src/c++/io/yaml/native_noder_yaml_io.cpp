#include "io/yaml/native_noder_yaml_io.hpp"

#include "array/array.hpp"
#include "array/factory/strings.hpp"

#include <algorithm>
#include <charconv>
#include <codecvt>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace io::yaml {

namespace {

struct YamlValue {
    enum class Kind {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using ObjectEntries = std::vector<std::pair<std::string, YamlValue>>;
    using ArrayEntries = std::vector<YamlValue>;

    Kind kind = Kind::Null;
    bool boolValue = false;
    std::string text;
    ArrayEntries arrayEntries;
    ObjectEntries objectEntries;

    static YamlValue makeNull() {
        return YamlValue{};
    }

    static YamlValue makeBool(const bool value) {
        YamlValue yamlValue;
        yamlValue.kind = Kind::Bool;
        yamlValue.boolValue = value;
        return yamlValue;
    }

    static YamlValue makeNumber(std::string value) {
        YamlValue yamlValue;
        yamlValue.kind = Kind::Number;
        yamlValue.text = std::move(value);
        return yamlValue;
    }

    static YamlValue makeString(std::string value) {
        YamlValue yamlValue;
        yamlValue.kind = Kind::String;
        yamlValue.text = std::move(value);
        return yamlValue;
    }

    static YamlValue makeArray(ArrayEntries value) {
        YamlValue yamlValue;
        yamlValue.kind = Kind::Array;
        yamlValue.arrayEntries = std::move(value);
        return yamlValue;
    }

    static YamlValue makeObject(ObjectEntries value) {
        YamlValue yamlValue;
        yamlValue.kind = Kind::Object;
        yamlValue.objectEntries = std::move(value);
        return yamlValue;
    }
};

struct YamlLine {
    size_t indent = 0;
    std::string text;
    size_t lineNumber = 0;
};

std::shared_ptr<void> allocate_bytes(const size_t byteCount) {
    if (byteCount == 0) {
        return nullptr;
    }

    return std::shared_ptr<void>(new std::uint8_t[byteCount](), [](void* ptr) {
        delete[] static_cast<std::uint8_t*>(ptr);
    });
}

std::string read_text_file(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error("io::yaml::read: could not open file '" + filename + "'");
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void write_text_file(const std::string& filename, const std::string& content) {
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("io::yaml::write_node: could not open file '" + filename + "' for writing");
    }

    output << content;
    if (!output.good()) {
        throw std::runtime_error("io::yaml::write_node: failed while writing '" + filename + "'");
    }
}

size_t checked_add(const size_t left, const size_t right, const char* context) {
    if (left > std::numeric_limits<size_t>::max() - right) {
        throw std::overflow_error(std::string(context) + ": size_t overflow");
    }
    return left + right;
}

size_t checked_mul(const size_t left, const size_t right, const char* context) {
    if (left != 0 && right > std::numeric_limits<size_t>::max() / left) {
        throw std::overflow_error(std::string(context) + ": size_t overflow");
    }
    return left * right;
}

size_t required_buffer_size(
    const std::vector<size_t>& shape,
    const std::vector<size_t>& strides,
    const size_t itemsize) {

    if (shape.size() != strides.size()) {
        throw std::invalid_argument("required_buffer_size: shape/strides rank mismatch");
    }
    if (shape.empty()) {
        return itemsize;
    }

    for (const size_t extent : shape) {
        if (extent == 0) {
            return 0;
        }
    }

    size_t maxOffset = 0;
    for (size_t i = 0; i < shape.size(); ++i) {
        const size_t stepCount = shape[i] - 1;
        maxOffset = checked_add(
            maxOffset,
            checked_mul(stepCount, strides[i], "required_buffer_size"),
            "required_buffer_size");
    }
    return checked_add(maxOffset, itemsize, "required_buffer_size");
}

size_t byte_offset_from_indices(const Array& array, const std::vector<size_t>& indices) {
    const std::vector<size_t> shape = array.shape();
    const std::vector<size_t> strides = array.strides();
    if (indices.size() != shape.size()) {
        throw std::invalid_argument("byte_offset_from_indices: wrong number of indices");
    }

    size_t offset = 0;
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i] >= shape[i]) {
            throw std::out_of_range("byte_offset_from_indices: index out of bounds");
        }
        offset = checked_add(
            offset,
            checked_mul(indices[i], strides[i], "byte_offset_from_indices"),
            "byte_offset_from_indices");
    }
    return offset;
}

std::string trim_copy(const std::string& value) {
    size_t begin = 0;
    while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t')) {
        begin += 1;
    }

    size_t end = value.size();
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r')) {
        end -= 1;
    }

    return value.substr(begin, end - begin);
}

std::string escape_double_quoted_yaml_string(const std::string& value) {
    std::ostringstream output;
    for (const char rawCharacter : value) {
        const unsigned char character = static_cast<unsigned char>(rawCharacter);
        switch (character) {
            case '\"':
                output << "\\\"";
                break;
            case '\\':
                output << "\\\\";
                break;
            case '\b':
                output << "\\b";
                break;
            case '\f':
                output << "\\f";
                break;
            case '\n':
                output << "\\n";
                break;
            case '\r':
                output << "\\r";
                break;
            case '\t':
                output << "\\t";
                break;
            default:
                if (character < 0x20U) {
                    output << "\\u"
                           << std::hex
                           << std::setw(4)
                           << std::setfill('0')
                           << static_cast<int>(character)
                           << std::dec
                           << std::setfill(' ');
                } else {
                    output << static_cast<char>(character);
                }
                break;
        }
    }
    return output.str();
}

bool is_plain_yaml_string(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    if (value == "null" || value == "true" || value == "false" || value == "~") {
        return false;
    }

    const unsigned char first = static_cast<unsigned char>(value.front());
    if (first == '-' || first == '?' || first == ':' || first == '[' || first == ']' ||
        first == '{' || first == '}' || first == ',' || first == '#' || first == '&' ||
        first == '*' || first == '!' || first == '|' || first == '>' || first == '@' ||
        first == '`' || first == '\'' || first == '\"' || first == '%' || first == ' ') {
        return false;
    }

    for (const char rawCharacter : value) {
        const unsigned char character = static_cast<unsigned char>(rawCharacter);
        if (character == ':' || character == '#' || character == ',' ||
            character == '[' || character == ']' || character == '{' || character == '}' ||
            character == '\n' || character == '\r' || character == '\t') {
            return false;
        }
        if (character == ' ') {
            return false;
        }
    }
    return true;
}

bool is_primitive_yaml(const YamlValue& value) {
    return value.kind == YamlValue::Kind::Null
        || value.kind == YamlValue::Kind::Bool
        || value.kind == YamlValue::Kind::Number
        || value.kind == YamlValue::Kind::String;
}

bool can_render_yaml_flow(const YamlValue& value) {
    if (value.kind != YamlValue::Kind::Array) {
        return false;
    }
    if (value.arrayEntries.empty()) {
        return true;
    }
    if (value.arrayEntries.size() > 8) {
        return false;
    }

    return std::all_of(
        value.arrayEntries.begin(),
        value.arrayEntries.end(),
        [](const YamlValue& entry) { return is_primitive_yaml(entry); });
}

std::string render_yaml_inline_value(const YamlValue& value);

std::string render_yaml_flow_array(const YamlValue::ArrayEntries& values) {
    std::ostringstream output;
    output << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            output << ", ";
        }
        output << render_yaml_inline_value(values[i]);
    }
    output << "]";
    return output.str();
}

std::string render_yaml_inline_value(const YamlValue& value) {
    switch (value.kind) {
        case YamlValue::Kind::Null:
            return "null";
        case YamlValue::Kind::Bool:
            return value.boolValue ? "true" : "false";
        case YamlValue::Kind::Number:
            return value.text;
        case YamlValue::Kind::String:
            if (is_plain_yaml_string(value.text)) {
                return value.text;
            }
            return "\"" + escape_double_quoted_yaml_string(value.text) + "\"";
        case YamlValue::Kind::Array:
            if (can_render_yaml_flow(value)) {
                return render_yaml_flow_array(value.arrayEntries);
            }
            break;
        case YamlValue::Kind::Object:
            break;
    }

    throw std::runtime_error("render_yaml_inline_value: value cannot be rendered inline");
}

void write_yaml_block(std::ostream& output, const YamlValue& value, const size_t indentLevel);

void write_yaml_indent(std::ostream& output, const size_t indentLevel) {
    for (size_t i = 0; i < indentLevel; ++i) {
        output << "  ";
    }
}

void write_yaml_object_block(
    std::ostream& output,
    const YamlValue::ObjectEntries& entries,
    const size_t indentLevel) {

    if (entries.empty()) {
        write_yaml_indent(output, indentLevel);
        output << "{}\n";
        return;
    }

    for (const auto& [key, value] : entries) {
        write_yaml_indent(output, indentLevel);
        output << key << ":";
        if (is_primitive_yaml(value) || can_render_yaml_flow(value)) {
            output << " " << render_yaml_inline_value(value) << "\n";
            continue;
        }
        output << "\n";
        write_yaml_block(output, value, indentLevel + 1);
    }
}

void write_yaml_array_block(
    std::ostream& output,
    const YamlValue::ArrayEntries& entries,
    const size_t indentLevel) {

    if (entries.empty()) {
        write_yaml_indent(output, indentLevel);
        output << "[]\n";
        return;
    }

    for (const YamlValue& value : entries) {
        write_yaml_indent(output, indentLevel);
        output << "-";
        if (is_primitive_yaml(value) || can_render_yaml_flow(value)) {
            output << " " << render_yaml_inline_value(value) << "\n";
            continue;
        }
        output << "\n";
        write_yaml_block(output, value, indentLevel + 1);
    }
}

void write_yaml_block(std::ostream& output, const YamlValue& value, const size_t indentLevel) {
    switch (value.kind) {
        case YamlValue::Kind::Object:
            write_yaml_object_block(output, value.objectEntries, indentLevel);
            return;
        case YamlValue::Kind::Array:
            write_yaml_array_block(output, value.arrayEntries, indentLevel);
            return;
        case YamlValue::Kind::Null:
        case YamlValue::Kind::Bool:
        case YamlValue::Kind::Number:
        case YamlValue::Kind::String:
            write_yaml_indent(output, indentLevel);
            output << render_yaml_inline_value(value) << "\n";
            return;
    }

    throw std::runtime_error("write_yaml_block: unsupported YAML kind");
}

std::string serialize_yaml(const YamlValue& value) {
    std::ostringstream output;
    write_yaml_block(output, value, 0);
    return output.str();
}

class FlowParser {
public:
    explicit FlowParser(std::string_view inputText) :
        _input(inputText) {}

    YamlValue parse() {
        YamlValue value = parse_value();
        skip_whitespace();
        if (_position != _input.size()) {
            throw std::runtime_error("io::yaml::read: unexpected trailing inline content");
        }
        return value;
    }

private:
    std::string_view _input;
    size_t _position = 0;

    void skip_whitespace() {
        while (_position < _input.size()) {
            const unsigned char value = static_cast<unsigned char>(_input[_position]);
            if (value != ' ' && value != '\t' && value != '\n' && value != '\r') {
                return;
            }
            _position += 1;
        }
    }

    char peek() const {
        if (_position >= _input.size()) {
            throw std::runtime_error("io::yaml::read: unexpected end of inline value");
        }
        return _input[_position];
    }

    char consume() {
        const char value = peek();
        _position += 1;
        return value;
    }

    void expect(const char expected) {
        const char value = consume();
        if (value != expected) {
            throw std::runtime_error(
                "io::yaml::read: expected '" + std::string(1, expected) + "', got '" + std::string(1, value) + "'");
        }
    }

    static bool is_hex_digit(const char value) {
        return (value >= '0' && value <= '9')
            || (value >= 'a' && value <= 'f')
            || (value >= 'A' && value <= 'F');
    }

    static unsigned hex_value(const char value) {
        if (value >= '0' && value <= '9') {
            return static_cast<unsigned>(value - '0');
        }
        if (value >= 'a' && value <= 'f') {
            return 10U + static_cast<unsigned>(value - 'a');
        }
        if (value >= 'A' && value <= 'F') {
            return 10U + static_cast<unsigned>(value - 'A');
        }
        throw std::runtime_error("io::yaml::read: invalid hexadecimal digit");
    }

    unsigned parse_hex_quad() {
        unsigned value = 0U;
        for (size_t i = 0; i < 4; ++i) {
            const char digit = consume();
            if (!is_hex_digit(digit)) {
                throw std::runtime_error("io::yaml::read: invalid \\u escape");
            }
            value = (value << 4U) | hex_value(digit);
        }
        return value;
    }

    static void append_utf8(std::string& output, const std::uint32_t codePoint) {
        if (codePoint <= 0x7FU) {
            output.push_back(static_cast<char>(codePoint));
            return;
        }
        if (codePoint <= 0x7FFU) {
            output.push_back(static_cast<char>(0xC0U | ((codePoint >> 6U) & 0x1FU)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
            return;
        }
        if (codePoint <= 0xFFFFU) {
            output.push_back(static_cast<char>(0xE0U | ((codePoint >> 12U) & 0x0FU)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
            return;
        }
        if (codePoint <= 0x10FFFFU) {
            output.push_back(static_cast<char>(0xF0U | ((codePoint >> 18U) & 0x07U)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 12U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
            return;
        }
        throw std::runtime_error("io::yaml::read: invalid Unicode code point");
    }

    std::string parse_string() {
        expect('"');

        std::string output;
        while (true) {
            if (_position >= _input.size()) {
                throw std::runtime_error("io::yaml::read: unterminated quoted string");
            }

            const char value = consume();
            if (value == '"') {
                return output;
            }
            if (value == '\\') {
                const char escaped = consume();
                switch (escaped) {
                    case '"':
                    case '\\':
                    case '/':
                        output.push_back(escaped);
                        break;
                    case 'b':
                        output.push_back('\b');
                        break;
                    case 'f':
                        output.push_back('\f');
                        break;
                    case 'n':
                        output.push_back('\n');
                        break;
                    case 'r':
                        output.push_back('\r');
                        break;
                    case 't':
                        output.push_back('\t');
                        break;
                    case 'u': {
                        std::uint32_t codePoint = parse_hex_quad();
                        if (codePoint >= 0xD800U && codePoint <= 0xDBFFU) {
                            expect('\\');
                            expect('u');
                            const std::uint32_t lowSurrogate = parse_hex_quad();
                            if (lowSurrogate < 0xDC00U || lowSurrogate > 0xDFFFU) {
                                throw std::runtime_error("io::yaml::read: invalid Unicode surrogate pair");
                            }
                            codePoint = 0x10000U + ((codePoint - 0xD800U) << 10U) + (lowSurrogate - 0xDC00U);
                        }
                        append_utf8(output, codePoint);
                        break;
                    }
                    default:
                        throw std::runtime_error("io::yaml::read: invalid escape sequence");
                }
                continue;
            }

            if (static_cast<unsigned char>(value) < 0x20U) {
                throw std::runtime_error("io::yaml::read: control character in quoted string");
            }
            output.push_back(value);
        }
    }

    YamlValue parse_array() {
        expect('[');
        skip_whitespace();

        YamlValue::ArrayEntries values;
        if (_position < _input.size() && _input[_position] == ']') {
            _position += 1;
            return YamlValue::makeArray(std::move(values));
        }

        while (true) {
            values.push_back(parse_value());
            skip_whitespace();

            const char next = consume();
            if (next == ']') {
                return YamlValue::makeArray(std::move(values));
            }
            if (next != ',') {
                throw std::runtime_error("io::yaml::read: expected ',' or ']'");
            }
            skip_whitespace();
        }
    }

    YamlValue parse_number() {
        const size_t start = _position;
        if (peek() == '-' || peek() == '+') {
            _position += 1;
        }

        if (_position >= _input.size()) {
            throw std::runtime_error("io::yaml::read: incomplete number");
        }

        if (_input[_position] == '0') {
            _position += 1;
        } else {
            if (_input[_position] < '1' || _input[_position] > '9') {
                throw std::runtime_error("io::yaml::read: invalid number");
            }
            while (_position < _input.size() && _input[_position] >= '0' && _input[_position] <= '9') {
                _position += 1;
            }
        }

        if (_position < _input.size() && _input[_position] == '.') {
            _position += 1;
            if (_position >= _input.size() || _input[_position] < '0' || _input[_position] > '9') {
                throw std::runtime_error("io::yaml::read: invalid fractional part");
            }
            while (_position < _input.size() && _input[_position] >= '0' && _input[_position] <= '9') {
                _position += 1;
            }
        }

        if (_position < _input.size() && (_input[_position] == 'e' || _input[_position] == 'E')) {
            _position += 1;
            if (_position < _input.size() && (_input[_position] == '+' || _input[_position] == '-')) {
                _position += 1;
            }
            if (_position >= _input.size() || _input[_position] < '0' || _input[_position] > '9') {
                throw std::runtime_error("io::yaml::read: invalid exponent");
            }
            while (_position < _input.size() && _input[_position] >= '0' && _input[_position] <= '9') {
                _position += 1;
            }
        }

        return YamlValue::makeNumber(std::string(_input.substr(start, _position - start)));
    }

    YamlValue parse_plain_scalar() {
        const size_t start = _position;
        while (_position < _input.size()) {
            const char value = _input[_position];
            if (value == ',' || value == ']' || value == ' ' || value == '\t' || value == '\n' || value == '\r') {
                break;
            }
            _position += 1;
        }

        if (start == _position) {
            throw std::runtime_error("io::yaml::read: expected inline scalar");
        }

        const std::string token(_input.substr(start, _position - start));
        if (token == "null" || token == "~") {
            return YamlValue::makeNull();
        }
        if (token == "true") {
            return YamlValue::makeBool(true);
        }
        if (token == "false") {
            return YamlValue::makeBool(false);
        }
        return YamlValue::makeString(token);
    }

    YamlValue parse_literal(const std::string_view literal, const YamlValue& value) {
        if (_input.substr(_position, literal.size()) != literal) {
            throw std::runtime_error("io::yaml::read: invalid literal");
        }
        _position += literal.size();
        return value;
    }

    YamlValue parse_value() {
        skip_whitespace();
        switch (peek()) {
            case 'n':
                return parse_literal("null", YamlValue::makeNull());
            case 't':
                return parse_literal("true", YamlValue::makeBool(true));
            case 'f':
                return parse_literal("false", YamlValue::makeBool(false));
            case '"':
                return YamlValue::makeString(parse_string());
            case '[':
                return parse_array();
            default:
                break;
        }

        const char first = peek();
        if (first == '-' || first == '+' || (first >= '0' && first <= '9')) {
            return parse_number();
        }
        return parse_plain_scalar();
    }
};

bool is_sequence_line(const std::string& text) {
    return !text.empty() && text.front() == '-' && (text.size() == 1 || text[1] == ' ');
}

std::vector<YamlLine> tokenize_yaml(const std::string& input) {
    std::vector<YamlLine> lines;
    std::istringstream stream(input);
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(stream, line)) {
        lineNumber += 1;

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t leadingSpaces = 0;
        while (leadingSpaces < line.size() && line[leadingSpaces] == ' ') {
            leadingSpaces += 1;
        }
        if (leadingSpaces < line.size() && line[leadingSpaces] == '\t') {
            throw std::runtime_error("io::yaml::read: tabs are not supported for indentation");
        }
        if (leadingSpaces % 2 != 0) {
            throw std::runtime_error("io::yaml::read: indentation must use multiples of two spaces");
        }

        const std::string text = trim_copy(line.substr(leadingSpaces));
        if (text.empty() || text.front() == '#') {
            continue;
        }

        lines.push_back(YamlLine{leadingSpaces / 2, text, lineNumber});
    }

    return lines;
}

YamlValue parse_inline_value(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty() || trimmed == "null" || trimmed == "~") {
        return YamlValue::makeNull();
    }
    if (trimmed == "true") {
        return YamlValue::makeBool(true);
    }
    if (trimmed == "false") {
        return YamlValue::makeBool(false);
    }
    if (trimmed.front() == '"' || trimmed.front() == '[' ||
        trimmed.front() == '-' || trimmed.front() == '+' ||
        (trimmed.front() >= '0' && trimmed.front() <= '9')) {
        try {
            return FlowParser(trimmed).parse();
        } catch (const std::runtime_error&) {
        }
    }

    return YamlValue::makeString(trimmed);
}

YamlValue parse_yaml_block(const std::vector<YamlLine>& lines, size_t& index, const size_t indent);

YamlValue parse_yaml_sequence(const std::vector<YamlLine>& lines, size_t& index, const size_t indent) {
    YamlValue::ArrayEntries values;

    while (index < lines.size() && lines[index].indent == indent && is_sequence_line(lines[index].text)) {
        const std::string rest = trim_copy(lines[index].text.substr(1));
        index += 1;

        if (rest.empty()) {
            if (index >= lines.size() || lines[index].indent <= indent) {
                values.push_back(YamlValue::makeNull());
                continue;
            }
            values.push_back(parse_yaml_block(lines, index, lines[index].indent));
            continue;
        }

        values.push_back(parse_inline_value(rest));
    }

    return YamlValue::makeArray(std::move(values));
}

YamlValue parse_yaml_object(const std::vector<YamlLine>& lines, size_t& index, const size_t indent) {
    YamlValue::ObjectEntries values;

    while (index < lines.size() && lines[index].indent == indent && !is_sequence_line(lines[index].text)) {
        const std::string& text = lines[index].text;
        const size_t colonPos = text.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error(
                "io::yaml::read: expected mapping entry on line " + std::to_string(lines[index].lineNumber));
        }

        const std::string key = trim_copy(text.substr(0, colonPos));
        const std::string rest = trim_copy(text.substr(colonPos + 1));
        index += 1;

        if (rest.empty()) {
            if (index >= lines.size() || lines[index].indent <= indent) {
                values.emplace_back(key, YamlValue::makeNull());
                continue;
            }
            values.emplace_back(key, parse_yaml_block(lines, index, lines[index].indent));
            continue;
        }

        values.emplace_back(key, parse_inline_value(rest));
    }

    return YamlValue::makeObject(std::move(values));
}

YamlValue parse_yaml_block(const std::vector<YamlLine>& lines, size_t& index, const size_t indent) {
    if (index >= lines.size()) {
        throw std::runtime_error("io::yaml::read: unexpected end of YAML document");
    }
    if (lines[index].indent != indent) {
        throw std::runtime_error(
            "io::yaml::read: inconsistent indentation on line " + std::to_string(lines[index].lineNumber));
    }

    if (is_sequence_line(lines[index].text)) {
        return parse_yaml_sequence(lines, index, indent);
    }
    return parse_yaml_object(lines, index, indent);
}

YamlValue parse_yaml(const std::string& text) {
    const std::vector<YamlLine> lines = tokenize_yaml(text);
    if (lines.empty()) {
        throw std::runtime_error("io::yaml::read: YAML document is empty");
    }

    size_t index = 0;
    YamlValue value = parse_yaml_block(lines, index, lines.front().indent);
    if (index != lines.size()) {
        throw std::runtime_error(
            "io::yaml::read: unexpected trailing YAML content on line " + std::to_string(lines[index].lineNumber));
    }
    return value;
}

const YamlValue* find_object_member(const YamlValue& objectValue, const std::string& key) {
    if (objectValue.kind != YamlValue::Kind::Object) {
        return nullptr;
    }

    for (const auto& entry : objectValue.objectEntries) {
        if (entry.first == key) {
            return &entry.second;
        }
    }
    return nullptr;
}

const YamlValue* require_object_member(const YamlValue& objectValue, const std::string& key, const char* context) {
    const YamlValue* member = find_object_member(objectValue, key);
    if (!member) {
        throw std::runtime_error(std::string(context) + ": missing member '" + key + "'");
    }
    return member;
}

YamlValue yaml_number_from_size(const size_t value) {
    return YamlValue::makeNumber(std::to_string(value));
}

template <typename T>
std::string finite_number_string(const T value) {
    if constexpr (std::is_floating_point_v<T>) {
        std::ostringstream output;
        output << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
        return output.str();
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else {
        return std::to_string(value);
    }
}

template <typename T>
YamlValue yaml_scalar_from_numeric(const T value) {
    if constexpr (std::is_same_v<T, bool>) {
        return YamlValue::makeBool(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(value)) {
            return YamlValue::makeString("NaN");
        }
        if (std::isinf(value)) {
            return YamlValue::makeString(value > 0 ? "Infinity" : "-Infinity");
        }
        return YamlValue::makeNumber(finite_number_string(value));
    } else {
        return YamlValue::makeNumber(finite_number_string(value));
    }
}

template <typename T>
YamlValue numeric_element_to_yaml(const Array& array, const size_t offset) {
    const auto* rawBytes = static_cast<const std::uint8_t*>(array.rawData());
    const T value = *reinterpret_cast<const T*>(rawBytes + offset);
    return yaml_scalar_from_numeric(value);
}

std::string bytes_element_to_string(const Array& array, const size_t offset) {
    const auto* rawBytes = static_cast<const std::uint8_t*>(array.rawData());
    std::string value(reinterpret_cast<const char*>(rawBytes + offset), array.itemsize());
    while (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }
    return value;
}

std::string unicode_element_to_string(const Array& array, const size_t offset) {
    if (array.itemsize() % sizeof(char32_t) != 0) {
        throw std::runtime_error("unicode_element_to_string: invalid unicode itemsize");
    }

    const auto* rawBytes = static_cast<const std::uint8_t*>(array.rawData());
    const auto* rawCodePoints = reinterpret_cast<const char32_t*>(rawBytes + offset);
    const size_t capacity = array.itemsize() / sizeof(char32_t);
    std::u32string codePoints;
    codePoints.reserve(capacity);
    for (size_t i = 0; i < capacity; ++i) {
        const char32_t codePoint = rawCodePoints[i];
        if (codePoint == U'\0') {
            break;
        }
        codePoints.push_back(codePoint);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.to_bytes(codePoints);
}

YamlValue array_value_to_yaml(
    const Array& array,
    const size_t dimension,
    std::vector<size_t>& indices) {

    const std::vector<size_t> shape = array.shape();
    if (dimension == shape.size()) {
        const size_t offset = byte_offset_from_indices(array, indices);
        switch (array.typeId()) {
            case ArrayTypeId::Bool:
                return numeric_element_to_yaml<bool>(array, offset);
            case ArrayTypeId::Int8:
                return numeric_element_to_yaml<int8_t>(array, offset);
            case ArrayTypeId::Int16:
                return numeric_element_to_yaml<int16_t>(array, offset);
            case ArrayTypeId::Int32:
                return numeric_element_to_yaml<int32_t>(array, offset);
            case ArrayTypeId::Int64:
                return numeric_element_to_yaml<int64_t>(array, offset);
            case ArrayTypeId::UInt8:
                return numeric_element_to_yaml<uint8_t>(array, offset);
            case ArrayTypeId::UInt16:
                return numeric_element_to_yaml<uint16_t>(array, offset);
            case ArrayTypeId::UInt32:
                return numeric_element_to_yaml<uint32_t>(array, offset);
            case ArrayTypeId::UInt64:
                return numeric_element_to_yaml<uint64_t>(array, offset);
            case ArrayTypeId::Float32:
                return numeric_element_to_yaml<float>(array, offset);
            case ArrayTypeId::Float64:
                return numeric_element_to_yaml<double>(array, offset);
            case ArrayTypeId::Bytes:
                return YamlValue::makeString(bytes_element_to_string(array, offset));
            case ArrayTypeId::Unicode:
                return YamlValue::makeString(unicode_element_to_string(array, offset));
            case ArrayTypeId::None:
                return YamlValue::makeNull();
        }

        throw std::runtime_error("array_value_to_yaml: unsupported array dtype");
    }

    YamlValue::ArrayEntries values;
    values.reserve(shape[dimension]);
    for (size_t i = 0; i < shape[dimension]; ++i) {
        indices.push_back(i);
        values.push_back(array_value_to_yaml(array, dimension + 1, indices));
        indices.pop_back();
    }
    return YamlValue::makeArray(std::move(values));
}

YamlValue size_vector_to_yaml(const std::vector<size_t>& values) {
    YamlValue::ArrayEntries yamlValues;
    yamlValues.reserve(values.size());
    for (const size_t value : values) {
        yamlValues.push_back(yaml_number_from_size(value));
    }
    return YamlValue::makeArray(std::move(yamlValues));
}

YamlValue array_to_yaml(const Array& array) {
    std::vector<size_t> indices;
    YamlValue::ObjectEntries entries;
    entries.emplace_back("dtype", YamlValue::makeString(array.dtype()));
    entries.emplace_back("itemsize", yaml_number_from_size(array.itemsize()));
    entries.emplace_back("shape", size_vector_to_yaml(array.shape()));
    entries.emplace_back("strides", size_vector_to_yaml(array.strides()));
    entries.emplace_back("values", array_value_to_yaml(array, 0, indices));
    return YamlValue::makeObject(std::move(entries));
}

YamlValue node_to_yaml(const std::shared_ptr<Node>& node) {
    if (!node) {
        throw std::invalid_argument("node_to_yaml: node cannot be null");
    }

    YamlValue::ObjectEntries entries;
    entries.emplace_back("name", YamlValue::makeString(node->name()));
    entries.emplace_back("type", YamlValue::makeString(node->type()));

    if (node->hasLinkTarget()) {
        YamlValue::ObjectEntries linkEntries;
        linkEntries.emplace_back("file", YamlValue::makeString(node->linkTargetFile()));
        linkEntries.emplace_back("path", YamlValue::makeString(node->linkTargetPath()));
        entries.emplace_back("link", YamlValue::makeObject(std::move(linkEntries)));
        entries.emplace_back("data", YamlValue::makeNull());
    } else if (node->noData()) {
        entries.emplace_back("data", YamlValue::makeNull());
    } else {
        auto array = std::dynamic_pointer_cast<Array>(node->dataPtr());
        if (!array) {
            throw std::runtime_error("node_to_yaml: unsupported Data subclass");
        }
        entries.emplace_back("data", array_to_yaml(*array));
    }

    YamlValue::ArrayEntries childrenEntries;
    childrenEntries.reserve(node->children().size());
    for (const auto& child : node->children()) {
        childrenEntries.push_back(node_to_yaml(child));
    }
    entries.emplace_back("children", YamlValue::makeArray(std::move(childrenEntries)));
    return YamlValue::makeObject(std::move(entries));
}

YamlValue document_to_yaml(const std::shared_ptr<Node>& node) {
    YamlValue::ObjectEntries entries;
    entries.emplace_back("format", YamlValue::makeString("noder-yaml"));
    entries.emplace_back("version", YamlValue::makeNumber("1"));
    entries.emplace_back("root", node_to_yaml(node));
    return YamlValue::makeObject(std::move(entries));
}

ArrayTypeId type_id_from_dtype_name(const std::string& dtypeName) {
    if (dtypeName == "bool") return ArrayTypeId::Bool;
    if (dtypeName == "int8") return ArrayTypeId::Int8;
    if (dtypeName == "int16") return ArrayTypeId::Int16;
    if (dtypeName == "int32") return ArrayTypeId::Int32;
    if (dtypeName == "int64") return ArrayTypeId::Int64;
    if (dtypeName == "uint8") return ArrayTypeId::UInt8;
    if (dtypeName == "uint16") return ArrayTypeId::UInt16;
    if (dtypeName == "uint32") return ArrayTypeId::UInt32;
    if (dtypeName == "uint64") return ArrayTypeId::UInt64;
    if (dtypeName == "float32") return ArrayTypeId::Float32;
    if (dtypeName == "float64") return ArrayTypeId::Float64;
    if (dtypeName == "bytes") return ArrayTypeId::Bytes;
    if (dtypeName == "unicode") return ArrayTypeId::Unicode;
    if (dtypeName == "none") return ArrayTypeId::None;
    throw std::runtime_error("type_id_from_dtype_name: unsupported dtype '" + dtypeName + "'");
}

template <typename IntegerType>
IntegerType parse_integer_string(const std::string& value, const char* context) {
    IntegerType parsedValue{};
    const char* begin = value.data();
    const char* end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(begin, end, parsedValue);
    if (ec != std::errc() || ptr != end) {
        throw std::runtime_error(std::string(context) + ": invalid integer '" + value + "'");
    }
    return parsedValue;
}

template <typename IntegerType>
IntegerType parse_integer_yaml(const YamlValue& value, const char* context) {
    if (value.kind != YamlValue::Kind::Number) {
        throw std::runtime_error(std::string(context) + ": expected numeric YAML value");
    }

    if constexpr (std::is_signed_v<IntegerType>) {
        const long long parsedValue = parse_integer_string<long long>(value.text, context);
        if (parsedValue < static_cast<long long>(std::numeric_limits<IntegerType>::min())
            || parsedValue > static_cast<long long>(std::numeric_limits<IntegerType>::max())) {
            throw std::runtime_error(std::string(context) + ": integer value out of range");
        }
        return static_cast<IntegerType>(parsedValue);
    } else {
        const unsigned long long parsedValue = parse_integer_string<unsigned long long>(value.text, context);
        if (parsedValue > static_cast<unsigned long long>(std::numeric_limits<IntegerType>::max())) {
            throw std::runtime_error(std::string(context) + ": integer value out of range");
        }
        return static_cast<IntegerType>(parsedValue);
    }
}

size_t parse_size_t(const YamlValue& value, const char* context) {
    if (value.kind != YamlValue::Kind::Number) {
        throw std::runtime_error(std::string(context) + ": expected numeric YAML value");
    }
    return parse_integer_yaml<size_t>(value, context);
}

std::vector<size_t> parse_size_vector(const YamlValue& value, const char* context) {
    if (value.kind != YamlValue::Kind::Array) {
        throw std::runtime_error(std::string(context) + ": expected YAML array");
    }

    std::vector<size_t> output;
    output.reserve(value.arrayEntries.size());
    for (const YamlValue& entry : value.arrayEntries) {
        output.push_back(parse_size_t(entry, context));
    }
    return output;
}

template <typename FloatType>
FloatType parse_float_yaml(const YamlValue& value, const char* context) {
    if (value.kind == YamlValue::Kind::String) {
        if (value.text == "NaN") {
            return std::numeric_limits<FloatType>::quiet_NaN();
        }
        if (value.text == "Infinity") {
            return std::numeric_limits<FloatType>::infinity();
        }
        if (value.text == "-Infinity") {
            return -std::numeric_limits<FloatType>::infinity();
        }
        throw std::runtime_error(std::string(context) + ": unsupported float string '" + value.text + "'");
    }
    if (value.kind != YamlValue::Kind::Number) {
        throw std::runtime_error(std::string(context) + ": expected numeric YAML value");
    }

    std::stringstream stream(value.text);
    FloatType parsedValue{};
    stream >> parsedValue;
    if (!stream || !stream.eof()) {
        throw std::runtime_error(std::string(context) + ": invalid floating-point value '" + value.text + "'");
    }
    return parsedValue;
}

bool parse_bool_yaml(const YamlValue& value, const char* context) {
    if (value.kind == YamlValue::Kind::Bool) {
        return value.boolValue;
    }
    if (value.kind == YamlValue::Kind::Number) {
        if (value.text == "0") {
            return false;
        }
        if (value.text == "1") {
            return true;
        }
    }
    throw std::runtime_error(std::string(context) + ": expected boolean YAML value");
}

Array make_array_storage(
    const ArrayTypeId typeId,
    const size_t itemsize,
    const std::vector<size_t>& shape,
    const std::vector<size_t>& strides) {

    const size_t byteCount = required_buffer_size(shape, strides, itemsize);
    auto owner = allocate_bytes(byteCount);

    if (typeId == ArrayTypeId::Bytes) {
        return Array::bytesView(owner.get(), itemsize, shape, strides, owner);
    }
    if (typeId == ArrayTypeId::Unicode) {
        return Array::unicodeView(owner.get(), itemsize, shape, strides, owner);
    }
    return Array(typeId, itemsize, owner.get(), shape, strides, owner);
}

template <typename T>
void store_numeric_value(Array& array, const size_t offset, const T value) {
    auto* rawBytes = static_cast<std::uint8_t*>(array.rawData());
    *reinterpret_cast<T*>(rawBytes + offset) = value;
}

void store_bytes_value(Array& array, const size_t offset, const std::string& value, const char* context) {
    if (value.size() > array.itemsize()) {
        throw std::runtime_error(std::string(context) + ": byte string exceeds itemsize");
    }

    auto* rawBytes = static_cast<std::uint8_t*>(array.rawData());
    std::memset(rawBytes + offset, 0, array.itemsize());
    if (!value.empty()) {
        std::memcpy(rawBytes + offset, value.data(), value.size());
    }
}

void store_unicode_value(Array& array, const size_t offset, const std::string& value, const char* context) {
    if (array.itemsize() % sizeof(char32_t) != 0) {
        throw std::runtime_error(std::string(context) + ": invalid unicode itemsize");
    }

    const std::u32string codePoints = arrayfactory::u32stringFromString(value);
    const size_t capacity = array.itemsize() / sizeof(char32_t);
    if (codePoints.size() > capacity) {
        throw std::runtime_error(std::string(context) + ": unicode string exceeds itemsize");
    }

    auto* rawBytes = static_cast<std::uint8_t*>(array.rawData());
    std::memset(rawBytes + offset, 0, array.itemsize());
    std::memcpy(rawBytes + offset, codePoints.data(), codePoints.size() * sizeof(char32_t));
}

void fill_array_values(
    Array& array,
    const YamlValue& value,
    const size_t dimension,
    std::vector<size_t>& indices,
    const char* context) {

    const std::vector<size_t> shape = array.shape();
    if (dimension == shape.size()) {
        const size_t offset = byte_offset_from_indices(array, indices);
        switch (array.typeId()) {
            case ArrayTypeId::Bool:
                store_numeric_value<bool>(array, offset, parse_bool_yaml(value, context));
                return;
            case ArrayTypeId::Int8:
                store_numeric_value<int8_t>(array, offset, parse_integer_yaml<int8_t>(value, context));
                return;
            case ArrayTypeId::Int16:
                store_numeric_value<int16_t>(array, offset, parse_integer_yaml<int16_t>(value, context));
                return;
            case ArrayTypeId::Int32:
                store_numeric_value<int32_t>(array, offset, parse_integer_yaml<int32_t>(value, context));
                return;
            case ArrayTypeId::Int64:
                store_numeric_value<int64_t>(array, offset, parse_integer_yaml<int64_t>(value, context));
                return;
            case ArrayTypeId::UInt8:
                store_numeric_value<uint8_t>(array, offset, parse_integer_yaml<uint8_t>(value, context));
                return;
            case ArrayTypeId::UInt16:
                store_numeric_value<uint16_t>(array, offset, parse_integer_yaml<uint16_t>(value, context));
                return;
            case ArrayTypeId::UInt32:
                store_numeric_value<uint32_t>(array, offset, parse_integer_yaml<uint32_t>(value, context));
                return;
            case ArrayTypeId::UInt64:
                store_numeric_value<uint64_t>(array, offset, parse_integer_yaml<uint64_t>(value, context));
                return;
            case ArrayTypeId::Float32:
                store_numeric_value<float>(array, offset, parse_float_yaml<float>(value, context));
                return;
            case ArrayTypeId::Float64:
                store_numeric_value<double>(array, offset, parse_float_yaml<double>(value, context));
                return;
            case ArrayTypeId::Bytes:
                if (value.kind != YamlValue::Kind::String) {
                    throw std::runtime_error(std::string(context) + ": expected string YAML value");
                }
                store_bytes_value(array, offset, value.text, context);
                return;
            case ArrayTypeId::Unicode:
                if (value.kind != YamlValue::Kind::String) {
                    throw std::runtime_error(std::string(context) + ": expected string YAML value");
                }
                store_unicode_value(array, offset, value.text, context);
                return;
            case ArrayTypeId::None:
                return;
        }

        throw std::runtime_error(std::string(context) + ": unsupported dtype");
    }

    if (value.kind != YamlValue::Kind::Array) {
        throw std::runtime_error(std::string(context) + ": expected nested YAML array");
    }
    if (value.arrayEntries.size() != shape[dimension]) {
        throw std::runtime_error(std::string(context) + ": shape mismatch while reading YAML array values");
    }

    for (size_t i = 0; i < value.arrayEntries.size(); ++i) {
        indices.push_back(i);
        fill_array_values(array, value.arrayEntries[i], dimension + 1, indices, context);
        indices.pop_back();
    }
}

std::shared_ptr<Data> array_from_yaml(const YamlValue& value) {
    if (value.kind != YamlValue::Kind::Object) {
        throw std::runtime_error("array_from_yaml: expected YAML object");
    }

    const YamlValue* dtypeValue = require_object_member(value, "dtype", "array_from_yaml");
    const YamlValue* itemsizeValue = require_object_member(value, "itemsize", "array_from_yaml");
    const YamlValue* shapeValue = require_object_member(value, "shape", "array_from_yaml");
    const YamlValue* stridesValue = require_object_member(value, "strides", "array_from_yaml");
    const YamlValue* valuesValue = require_object_member(value, "values", "array_from_yaml");

    const std::string dtypeName = dtypeValue->text;
    const size_t itemsize = parse_size_t(*itemsizeValue, "array_from_yaml");
    const std::vector<size_t> shape = parse_size_vector(*shapeValue, "array_from_yaml");
    const std::vector<size_t> strides = parse_size_vector(*stridesValue, "array_from_yaml");

    if (shape.size() != strides.size()) {
        throw std::runtime_error("array_from_yaml: shape/strides rank mismatch");
    }

    const ArrayTypeId typeId = type_id_from_dtype_name(dtypeName);
    Array array = make_array_storage(typeId, itemsize, shape, strides);

    std::vector<size_t> indices;
    fill_array_values(array, *valuesValue, 0, indices, "array_from_yaml");
    return std::make_shared<Array>(array);
}

std::shared_ptr<Node> node_from_yaml(const YamlValue& value) {
    if (value.kind != YamlValue::Kind::Object) {
        throw std::runtime_error("node_from_yaml: expected YAML object");
    }

    const YamlValue* nameValue = require_object_member(value, "name", "node_from_yaml");
    const YamlValue* typeValue = require_object_member(value, "type", "node_from_yaml");
    if (nameValue->kind != YamlValue::Kind::String || typeValue->kind != YamlValue::Kind::String) {
        throw std::runtime_error("node_from_yaml: name/type must be strings");
    }

    auto node = std::make_shared<Node>(nameValue->text, typeValue->text);

    if (const YamlValue* linkValue = find_object_member(value, "link")) {
        if (linkValue->kind != YamlValue::Kind::Object) {
            throw std::runtime_error("node_from_yaml: link must be an object");
        }
        const YamlValue* fileValue = require_object_member(*linkValue, "file", "node_from_yaml");
        const YamlValue* pathValue = require_object_member(*linkValue, "path", "node_from_yaml");
        if (fileValue->kind != YamlValue::Kind::String || pathValue->kind != YamlValue::Kind::String) {
            throw std::runtime_error("node_from_yaml: link file/path must be strings");
        }
        node->setLinkTarget(fileValue->text, pathValue->text);
    } else {
        const YamlValue* dataValue = find_object_member(value, "data");
        if (dataValue && dataValue->kind != YamlValue::Kind::Null) {
            node->setData(array_from_yaml(*dataValue));
        }
    }

    const YamlValue* childrenValue = require_object_member(value, "children", "node_from_yaml");
    if (childrenValue->kind != YamlValue::Kind::Array) {
        throw std::runtime_error("node_from_yaml: children must be an array");
    }

    for (const YamlValue& childValue : childrenValue->arrayEntries) {
        node->addChild(node_from_yaml(childValue));
    }

    return node;
}

std::shared_ptr<Node> document_from_yaml(const YamlValue& value) {
    if (value.kind != YamlValue::Kind::Object) {
        throw std::runtime_error("io::yaml::read: document root must be a YAML object");
    }

    const YamlValue* formatValue = require_object_member(value, "format", "io::yaml::read");
    if (formatValue->kind != YamlValue::Kind::String || formatValue->text != "noder-yaml") {
        throw std::runtime_error("io::yaml::read: unsupported YAML document format");
    }

    const YamlValue* rootValue = require_object_member(value, "root", "io::yaml::read");
    return node_from_yaml(*rootValue);
}

} // namespace

void write_node(const std::string& filename, std::shared_ptr<Node> node) {
    if (!node) {
        throw std::invalid_argument("io::yaml::write_node: node cannot be null");
    }

    write_text_file(filename, serialize_yaml(document_to_yaml(node)));
}

std::shared_ptr<Node> read(const std::string& filename) {
    return document_from_yaml(parse_yaml(read_text_file(filename)));
}

} // namespace io::yaml
