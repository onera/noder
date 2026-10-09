#include "apps/cgnsviz/terminal_view/terminal_model.hpp"

#include "array/array.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"
#endif

#include <cctype>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <cerrno>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

#ifndef NODER_VERSION
#define NODER_VERSION "unknown"
#endif

namespace cgnsviz::terminal {

namespace {

std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string quoteCommandArgument(const std::string& value) {
#ifdef _WIN32
    std::string result = "\"";
    for (const char character : value) {
        if (character == '"') {
            result += "\\\"";
        } else {
            result.push_back(character);
        }
    }
    result.push_back('"');
    return result;
#else
    std::string result = "'";
    for (const char character : value) {
        if (character == '\'') {
            result += "'\\''";
        } else {
            result.push_back(character);
        }
    }
    result.push_back('\'');
    return result;
#endif
}

#ifdef _WIN32
std::string quoteWindowsProcessArgument(const std::string& value) {
    std::string result = "\"";
    std::size_t backslashes = 0;
    for (const char character : value) {
        if (character == '\\') {
            ++backslashes;
        } else if (character == '"') {
            result.append(backslashes * 2 + 1, '\\');
            result.push_back('"');
            backslashes = 0;
        } else {
            result.append(backslashes, '\\');
            result.push_back(character);
            backslashes = 0;
        }
    }
    result.append(backslashes * 2, '\\');
    result.push_back('"');
    return result;
}
#endif

int runPayloadEvaluator(const std::string& python,
                        const std::filesystem::path& expressionPath,
                        const std::string& filename,
                        const std::string& nodePath,
                        const std::filesystem::path& resultPath,
                        const std::string& order,
                        const std::filesystem::path& logPath) {
#ifdef _WIN32
    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;
    HANDLE logHandle = CreateFileA(
        logPath.string().c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &securityAttributes, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (logHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("could not create the Python payload evaluator log");
    }

    const std::vector<std::string> arguments = {
        python,
        "-m",
        "noder.payload_expression",
        "--cgnsviz-evaluate",
        expressionPath.string(),
        filename,
        nodePath,
        resultPath.string(),
        order,
    };
    std::string commandLine;
    for (const std::string& argument : arguments) {
        if (!commandLine.empty()) {
            commandLine.push_back(' ');
        }
        commandLine += quoteWindowsProcessArgument(argument);
    }

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = logHandle;
    startupInfo.hStdError = logHandle;
    PROCESS_INFORMATION processInfo{};
    const BOOL created = CreateProcessA(
        nullptr, commandLine.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo);
    const DWORD creationError = created ? ERROR_SUCCESS : GetLastError();
    CloseHandle(logHandle);
    if (!created) {
        throw std::runtime_error(
            "could not start the Python payload evaluator (Windows error " +
            std::to_string(creationError) + ")");
    }

    const DWORD waitResult = WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = 1;
    const BOOL gotExitCode = waitResult == WAIT_OBJECT_0 &&
        GetExitCodeProcess(processInfo.hProcess, &exitCode);
    const DWORD processError = gotExitCode ? ERROR_SUCCESS : GetLastError();
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    if (!gotExitCode) {
        throw std::runtime_error(
            "could not read the Python payload evaluator exit status (Windows error " +
            std::to_string(processError) + ")");
    }
    return static_cast<int>(exitCode);
#else
    const std::string command =
        quoteCommandArgument(python) + " -m noder.payload_expression --cgnsviz-evaluate " +
        quoteCommandArgument(expressionPath.string()) + " " +
        quoteCommandArgument(filename) + " " +
        quoteCommandArgument(nodePath) + " " +
        quoteCommandArgument(resultPath.string()) + " " +
        quoteCommandArgument(order) +
        " > " + quoteCommandArgument(logPath.string()) + " 2>&1";
    return std::system(command.c_str());
#endif
}

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned long long> sequence{0};
#ifdef _WIN32
        const auto processId = static_cast<unsigned long long>(GetCurrentProcessId());
#else
        const auto processId = static_cast<unsigned long long>(::getpid());
#endif
        const auto stamp = static_cast<unsigned long long>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        _path = std::filesystem::temp_directory_path() /
            ("noder-cgnsviz-edit-" + std::to_string(processId) + "-" +
             std::to_string(stamp) + "-" + std::to_string(sequence++));
        if (!std::filesystem::create_directory(_path)) {
            throw std::runtime_error("could not create a temporary payload-edit directory");
        }
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    const std::filesystem::path& path() const { return _path; }

private:
    std::filesystem::path _path;
};

std::uint32_t readLittleEndian(std::istream& input, const std::size_t byteCount) {
    std::uint32_t value = 0;
    for (std::size_t index = 0; index < byteCount; ++index) {
        const int byte = input.get();
        if (byte == std::char_traits<char>::eof()) {
            throw std::runtime_error("truncated NumPy payload result");
        }
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte))
            << (8 * index);
    }
    return value;
}

std::vector<std::size_t> parseNpyShape(const std::string& shapeText) {
    std::vector<std::size_t> shape;
    std::istringstream values(shapeText);
    std::string value;
    while (std::getline(values, value, ',')) {
        value = trim(value);
        if (!value.empty()) {
            std::size_t parsed = 0;
            const unsigned long long extent = std::stoull(value, &parsed);
            if (parsed != value.size() || extent > std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("invalid NumPy payload shape");
            }
            shape.push_back(static_cast<std::size_t>(extent));
        }
    }
    return shape;
}

std::shared_ptr<Data> readPythonPayloadResult(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Python payload evaluator did not produce a result");
    }
    std::string marker;
    std::getline(input, marker);
    if (marker == "NODER_NONE") {
        return std::make_shared<Array>();
    }
    input.clear();
    input.seekg(0);

    char magic[6]{};
    input.read(magic, sizeof(magic));
    if (!input || std::string(magic, sizeof(magic)) != std::string("\x93NUMPY", 6)) {
        throw std::runtime_error("Python payload evaluator returned an invalid result file");
    }
    const int major = input.get();
    const int minor = input.get();
    if (major < 1 || major > 3 || minor < 0) {
        throw std::runtime_error("unsupported NumPy payload result version");
    }
    const std::size_t lengthBytes = major == 1 ? 2 : 4;
    const std::uint32_t headerLength = readLittleEndian(input, lengthBytes);
    std::string header(headerLength, '\0');
    input.read(header.data(), static_cast<std::streamsize>(header.size()));
    if (!input) {
        throw std::runtime_error("truncated NumPy payload result header");
    }

    std::smatch match;
    const std::regex descrPattern(R"(['"]descr['"]\s*:\s*['"]([^'"]+)['"])");
    if (!std::regex_search(header, match, descrPattern)) {
        throw std::runtime_error("NumPy payload result has no dtype descriptor");
    }
    const std::string descriptor = match[1].str();
    const std::regex fortranPattern(R"(['"]fortran_order['"]\s*:\s*(True|False))");
    if (!std::regex_search(header, match, fortranPattern) || match[1].str() != "False") {
        throw std::runtime_error("NumPy payload result must use C memory order");
    }
    const std::regex shapePattern(R"(['"]shape['"]\s*:\s*\(([^)]*)\))");
    if (!std::regex_search(header, match, shapePattern)) {
        throw std::runtime_error("NumPy payload result has no shape");
    }
    const std::vector<std::size_t> shape = parseNpyShape(match[1].str());

    const std::regex typePattern(R"(^([<>=|]?)([biufSU])([0-9]+)$)");
    if (!std::regex_match(descriptor, match, typePattern)) {
        throw std::runtime_error("unsupported NumPy payload dtype '" + descriptor + "'");
    }
    const char byteOrder = match[1].str().empty() ? '=' : match[1].str()[0];
    const char kind = match[2].str()[0];
    const std::size_t typeWidth = static_cast<std::size_t>(std::stoull(match[3].str()));
    if (byteOrder == '>') {
        throw std::runtime_error("big-endian NumPy payload results are not supported");
    }

    ArrayTypeId typeId = ArrayTypeId::None;
    std::size_t itemSize = typeWidth;
    if (kind == 'b' && typeWidth == 1) typeId = ArrayTypeId::Bool;
    else if (kind == 'i' && typeWidth == 1) typeId = ArrayTypeId::Int8;
    else if (kind == 'i' && typeWidth == 2) typeId = ArrayTypeId::Int16;
    else if (kind == 'i' && typeWidth == 4) typeId = ArrayTypeId::Int32;
    else if (kind == 'i' && typeWidth == 8) typeId = ArrayTypeId::Int64;
    else if (kind == 'u' && typeWidth == 1) typeId = ArrayTypeId::UInt8;
    else if (kind == 'u' && typeWidth == 2) typeId = ArrayTypeId::UInt16;
    else if (kind == 'u' && typeWidth == 4) typeId = ArrayTypeId::UInt32;
    else if (kind == 'u' && typeWidth == 8) typeId = ArrayTypeId::UInt64;
    else if (kind == 'f' && typeWidth == 4) typeId = ArrayTypeId::Float32;
    else if (kind == 'f' && typeWidth == 8) typeId = ArrayTypeId::Float64;
    else if (kind == 'S') typeId = ArrayTypeId::Bytes;
    else if (kind == 'U') {
        typeId = ArrayTypeId::Unicode;
        if (typeWidth > std::numeric_limits<std::size_t>::max() / sizeof(char32_t)) {
            throw std::runtime_error("NumPy Unicode payload item size overflow");
        }
        itemSize *= sizeof(char32_t);
    } else {
        throw std::runtime_error("unsupported NumPy payload dtype '" + descriptor + "'");
    }

    std::size_t elementCount = 1;
    for (const std::size_t extent : shape) {
        if (extent != 0 && elementCount > std::numeric_limits<std::size_t>::max() / extent) {
            throw std::runtime_error("NumPy payload shape is too large");
        }
        elementCount *= extent;
    }
    if (itemSize != 0 && elementCount > std::numeric_limits<std::size_t>::max() / itemSize) {
        throw std::runtime_error("NumPy payload byte size is too large");
    }
    const std::size_t byteCount = elementCount * itemSize;
    std::shared_ptr<void> owner(new std::uint8_t[byteCount == 0 ? 1 : byteCount],
        [](void* value) { delete[] static_cast<std::uint8_t*>(value); });
    auto* buffer = static_cast<std::uint8_t*>(owner.get());
    input.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(byteCount));
    if (!input && byteCount > 0) {
        throw std::runtime_error("truncated NumPy payload data");
    }

    std::vector<std::size_t> strides(shape.size());
    std::size_t stride = itemSize;
    for (std::size_t dimension = shape.size(); dimension-- > 0;) {
        strides[dimension] = stride;
        if (shape[dimension] != 0 && stride > std::numeric_limits<std::size_t>::max() / shape[dimension]) {
            throw std::runtime_error("NumPy payload strides are too large");
        }
        stride *= shape[dimension];
    }
    return std::make_shared<Array>(
        typeId, itemSize, buffer, shape, strides, std::move(owner));
}

std::string readFileText(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream contents;
    contents << input.rdbuf();
    return trim(contents.str());
}

std::string loadStateName(const ChildrenLoadState state) {
    switch (state) {
        case ChildrenLoadState::Unloaded:
            return "unloaded";
        case ChildrenLoadState::Partial:
            return "partial";
        case ChildrenLoadState::Complete:
            return "complete";
    }
    return "unknown";
}

std::string compactMatchPath(const std::string& path) {
    const std::size_t firstSeparator = path.find('/');
    const std::size_t lastSeparator = path.rfind('/');
    if (firstSeparator == std::string::npos || firstSeparator == lastSeparator) {
        return {};
    }
    return path.substr(
        firstSeparator + 1,
        lastSeparator - firstSeparator - 1);
}

std::shared_ptr<Node> childAt(const std::shared_ptr<Node>& parent, const std::size_t index) {
    if (!parent) {
        return nullptr;
    }
    const auto& children = parent->loadedChildren();
    if (index >= children.size()) {
        return nullptr;
    }
    return children[index];
}

bool isNumericalArray(const Data& data) {
    const auto* array = dynamic_cast<const Array*>(&data);
    if (array == nullptr) {
        return false;
    }
    const char kind = array->dtypeKind();
    return kind == 'b' || kind == 'i' || kind == 'u' || kind == 'f';
}

template <typename T>
std::string numericalSummaryForType(const Array& array) {
    if (array.size() == 0) {
        return "empty array";
    }

    std::vector<T> values;
    values.reserve(array.size());

    long double sum = 0.0L;
    bool containsNaN = false;
    for (std::size_t index = 0; index < array.size(); ++index) {
        const T value = array.getItemAtIndex<T>(index);
        values.push_back(value);
        const long double numericValue = static_cast<long double>(value);
        containsNaN = containsNaN || std::isnan(numericValue);
        sum += numericValue;
    }

    if (containsNaN) {
        return "mn=nan MX=nan avg=nan med=nan";
    }

    std::sort(values.begin(), values.end());
    const auto minIterator = std::min_element(values.begin(), values.end());
    const auto maxIterator = std::max_element(values.begin(), values.end());
    const long double mean = sum / static_cast<long double>(values.size());
    long double median = static_cast<long double>(values[values.size() / 2]);
    if (values.size() % 2 == 0) {
        median = (
            static_cast<long double>(values[(values.size() / 2) - 1]) +
            static_cast<long double>(values[values.size() / 2])) / 2.0L;
    }

    std::ostringstream stream;
    stream << std::defaultfloat << std::setprecision(6);
    stream << "mn=" << *minIterator
           << " MX=" << *maxIterator
           << " avg=" << mean
           << " med=" << median;
    return stream.str();
}

std::string numericalSummary(const Array& array) {
    switch (array.typeId()) {
        case ArrayTypeId::Bool: return numericalSummaryForType<bool>(array);
        case ArrayTypeId::Int8: return numericalSummaryForType<int8_t>(array);
        case ArrayTypeId::Int16: return numericalSummaryForType<int16_t>(array);
        case ArrayTypeId::Int32: return numericalSummaryForType<int32_t>(array);
        case ArrayTypeId::Int64: return numericalSummaryForType<int64_t>(array);
        case ArrayTypeId::UInt8: return numericalSummaryForType<uint8_t>(array);
        case ArrayTypeId::UInt16: return numericalSummaryForType<uint16_t>(array);
        case ArrayTypeId::UInt32: return numericalSummaryForType<uint32_t>(array);
        case ArrayTypeId::UInt64: return numericalSummaryForType<uint64_t>(array);
        case ArrayTypeId::Float32: return numericalSummaryForType<float>(array);
        case ArrayTypeId::Float64: return numericalSummaryForType<double>(array);
        case ArrayTypeId::None:
        case ArrayTypeId::Bytes:
        case ArrayTypeId::Unicode:
            break;
    }
    throw std::invalid_argument("cannot summarize a non-numerical array");
}

std::string shapeText(const Data& data) {
    const std::vector<std::size_t> shape = data.shape();
    if (shape.empty()) {
        return "scalar";
    }

    std::ostringstream stream;
    for (std::size_t index = 0; index < shape.size(); ++index) {
        if (index > 0) {
            stream << 'x';
        }
        stream << shape[index];
    }
    return stream.str();
}

std::string numericalArrayText(const Array& array) {
    return "Array " + array.dtype() + " " + array.getPrintString(0);
}

std::string compactString(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    bool pendingSpace = false;
    for (const unsigned char character : value) {
        if (std::isspace(character) != 0) {
            pendingSpace = !result.empty();
            continue;
        }
        if (pendingSpace) {
            result.push_back(' ');
            pendingSpace = false;
        }
        result.push_back(static_cast<char>(character));
    }
    return result;
}

std::string shortWord(const std::string& word) {
    constexpr std::size_t maxWordCharacters = 24;
    if (word.size() <= maxWordCharacters) {
        return word;
    }
    return word.substr(0, maxWordCharacters - 3) + "...";
}

std::string payloadFileName(const std::string& nodeName) {
    std::string safeName;
    safeName.reserve(nodeName.size());
    for (const char character : nodeName) {
        const unsigned char unsignedCharacter = static_cast<unsigned char>(character);
        if (std::isalnum(unsignedCharacter) != 0 ||
            character == '-' || character == '_' || character == '.') {
            safeName.push_back(character);
        } else {
            safeName.push_back('_');
        }
    }
    if (safeName.empty()) {
        safeName = "unnamed";
    }
    return "cgnsviz-data-" + safeName + ".txt";
}

std::string stringMarkerText(const std::string& value, const std::size_t maxCharacters) {
    const std::string compact = compactString(value);
    if (value.size() <= maxCharacters) {
        return compact;
    }

    std::istringstream words(value);
    std::string first;
    std::string last;
    std::string word;
    std::size_t wordCount = 0;
    while (words >> word) {
        if (wordCount == 0) {
            first = word;
        }
        last = word;
        ++wordCount;
    }

    std::ostringstream summary;
    summary << "big str: " << wordCount << " words";
    if (wordCount > 0) {
        summary << " \"" << shortWord(first);
        if (wordCount > 1) {
            summary << " ... " << shortWord(last);
        }
        summary << "\"";
    }
    return summary.str();
}

template <typename T>
std::string numericalValueText(const T value) {
    std::ostringstream stream;
    stream << std::setprecision(15);
    if constexpr (std::is_same_v<T, bool>) {
        stream << (value ? 1 : 0);
    } else {
        stream << +value;
    }
    return stream.str();
}

template <typename T>
void appendNumericalPayloadLines(const Array& array, std::vector<std::string>& lines) {
    constexpr std::size_t lineWidth = 96;
    std::string line;
    for (std::size_t index = 0; index < array.size(); ++index) {
        const std::string value = numericalValueText(array.getItemAtIndex<T>(index));
        const std::size_t separatorSize = line.empty() ? 0 : 1;
        if (!line.empty() && line.size() + separatorSize + value.size() > lineWidth) {
            lines.push_back(line);
            line.clear();
        }
        if (!line.empty()) {
            line += ' ';
        }
        line += value;
    }
    if (line.empty()) {
        lines.emplace_back();
    } else {
        lines.push_back(line);
    }
}

std::vector<std::string> detailedPayloadLines(const Data& data) {
    const auto* array = dynamic_cast<const Array*>(&data);
    if (array != nullptr && isNumericalArray(data)) {
        std::vector<std::string> lines;
        switch (array->typeId()) {
            case ArrayTypeId::Bool: appendNumericalPayloadLines<bool>(*array, lines); break;
            case ArrayTypeId::Int8: appendNumericalPayloadLines<int8_t>(*array, lines); break;
            case ArrayTypeId::Int16: appendNumericalPayloadLines<int16_t>(*array, lines); break;
            case ArrayTypeId::Int32: appendNumericalPayloadLines<int32_t>(*array, lines); break;
            case ArrayTypeId::Int64: appendNumericalPayloadLines<int64_t>(*array, lines); break;
            case ArrayTypeId::UInt8: appendNumericalPayloadLines<uint8_t>(*array, lines); break;
            case ArrayTypeId::UInt16: appendNumericalPayloadLines<uint16_t>(*array, lines); break;
            case ArrayTypeId::UInt32: appendNumericalPayloadLines<uint32_t>(*array, lines); break;
            case ArrayTypeId::UInt64: appendNumericalPayloadLines<uint64_t>(*array, lines); break;
            case ArrayTypeId::Float32: appendNumericalPayloadLines<float>(*array, lines); break;
            case ArrayTypeId::Float64: appendNumericalPayloadLines<double>(*array, lines); break;
            case ArrayTypeId::None:
            case ArrayTypeId::Bytes:
            case ArrayTypeId::Unicode:
                break;
        }
        return lines;
    }

    const std::string value = data.hasString() ? data.extractString() : data.shortInfo();
    std::vector<std::string> lines;
    constexpr std::size_t lineWidth = 96;
    std::size_t lineStart = 0;
    while (lineStart <= value.size()) {
        const std::size_t newline = value.find('\n', lineStart);
        const std::size_t lineEnd = newline == std::string::npos ? value.size() : newline;
        if (lineStart == lineEnd) {
            lines.emplace_back();
        } else {
            for (std::size_t offset = lineStart; offset < lineEnd; offset += lineWidth) {
                lines.push_back(value.substr(offset, std::min(lineWidth, lineEnd - offset)));
            }
        }
        if (newline == std::string::npos) {
            break;
        }
        lineStart = newline + 1;
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

} // namespace

TerminalModel::TerminalModel(
    std::shared_ptr<io::hdf5::cgns::LazyHdf5Reader> reader,
    const std::size_t pageSize,
    const std::size_t payloadElementLimit,
    const std::size_t maxPayloadChars)
    : _reader(std::move(reader)),
      _current(nullptr),
      _selectedIndex(0),
      _firstVisibleIndex(0),
      _rootSelected(true),
      _pageSize(std::max<std::size_t>(1, pageSize)),
      _payloadElementLimit(payloadElementLimit),
      _maxPayloadChars(std::max<std::size_t>(1, maxPayloadChars)),
      _viewportRows(24),
      _statusMessage(),
      _viewMode(ViewMode::Node),
      _payloadNode(nullptr),
      _payloadLines(),
      _payloadScrollOffset(0),
      _searchResults(),
      _searchResultIndex(0),
      _firstVisibleMatchIndex(0),
      _searchExpression(),
      _malformedNodesFound(_reader && _reader->hasWarnings()) {

    if (!_reader) {
        throw std::invalid_argument("TerminalModel: reader cannot be null");
    }
    _current = _reader->root();
    if (!_current) {
        throw std::runtime_error("TerminalModel: lazy reader returned a null root");
    }
    ensureVisiblePage();
}

void TerminalModel::refreshMalformedWarning() const {
    if (_reader && _reader->hasWarnings()) {
        _malformedNodesFound = true;
    }
}

void TerminalModel::renderMalformedWarning(std::ostream& output) const {
    refreshMalformedWarning();
    if (_malformedNodesFound) {
        output << "\n\033[31mMalformed nodes where found during reading, search using / t:Corrupted_t\033[0m\n";
    }
}

void TerminalModel::setViewportRows(const std::size_t rows) {
    _viewportRows = std::max<std::size_t>(1, rows);
    switch (_viewMode) {
        case ViewMode::Node:
            ensureSelectionVisible();
            break;
        case ViewMode::Matches:
            ensureMatchSelectionVisible();
            break;
        case ViewMode::Payload:
            scrollPayload(0);
            break;
    }
}

std::size_t TerminalModel::childViewportRows() const {
    constexpr std::size_t reservedRows = 8;
    return _viewportRows > reservedRows ? _viewportRows - reservedRows : 1;
}

std::size_t TerminalModel::matchViewportRows() const {
    constexpr std::size_t reservedRows = 9;
    return _viewportRows > reservedRows ? _viewportRows - reservedRows : 1;
}

std::size_t TerminalModel::payloadViewportRows() const {
    // The regular payload layout uses six rows outside the data area:
    // title, path, metadata, two separators, and the footer.  On short
    // terminals renderPayload switches to a compact layout and keeps only
    // the three metadata rows, omitting the footer and separators.
    if (_viewportRows >= 7) {
        return _viewportRows - 6;
    }
    if (_viewportRows >= 4) {
        return _viewportRows - 3;
    }
    return 1;
}

void TerminalModel::ensureVisiblePage() {
    ensureSelectionVisible();
    const std::size_t requestedCount = std::max(_pageSize, childViewportRows());
    const std::size_t remaining = std::numeric_limits<std::size_t>::max() - _selectedIndex;
    _current->ensureChildrenLoaded(
        requestedCount > remaining ? std::numeric_limits<std::size_t>::max() : _selectedIndex + requestedCount);
}

void TerminalModel::ensureSelectionVisible() {
    const std::size_t visibleRows = childViewportRows();
    if (_selectedIndex < _firstVisibleIndex) {
        _firstVisibleIndex = _selectedIndex;
        return;
    }

    const std::size_t lastVisibleExclusive = _firstVisibleIndex >
            std::numeric_limits<std::size_t>::max() - visibleRows
        ? std::numeric_limits<std::size_t>::max()
        : _firstVisibleIndex + visibleRows;
    if (_selectedIndex >= lastVisibleExclusive) {
        _firstVisibleIndex = _selectedIndex - visibleRows + 1;
    }
}

std::shared_ptr<Node> TerminalModel::currentNode() const {
    return _current;
}

std::shared_ptr<Node> TerminalModel::selectedNode() const {
    if (_viewMode == ViewMode::Payload) {
        return _payloadNode;
    }
    if (_viewMode == ViewMode::Matches) {
        return selectedMatch();
    }
    if (_rootSelected) {
        return _current;
    }
    return childAt(_current, _selectedIndex);
}

std::size_t TerminalModel::selectedIndex() const {
    return _selectedIndex;
}

const std::string& TerminalModel::statusMessage() const {
    return _statusMessage;
}

void TerminalModel::selectRoot() {
    _current = _reader->root();
    _selectedIndex = 0;
    _firstVisibleIndex = 0;
    _rootSelected = true;
    _viewMode = ViewMode::Node;
    _payloadNode.reset();
    _payloadLines.clear();
    _payloadScrollOffset = 0;
    _statusMessage.clear();
    ensureVisiblePage();
}

std::shared_ptr<Node> TerminalModel::selectedMatch() const {
    if (_searchResultIndex >= _searchResults.size()) {
        return nullptr;
    }
    return _searchResults[_searchResultIndex];
}

void TerminalModel::selectNodeInNodeView(const std::shared_ptr<Node>& node) {
    if (!node) {
        return;
    }

    const std::shared_ptr<Node> parent = node->parent().lock();
    if (!parent) {
        _current = node;
        _selectedIndex = 0;
        _firstVisibleIndex = 0;
        _rootSelected = true;
        ensureVisiblePage();
        return;
    }

    parent->ensureChildrenLoaded();
    const auto& siblings = parent->loadedChildren();
    const auto iterator = std::find_if(
        siblings.begin(),
        siblings.end(),
        [&node](const std::shared_ptr<Node>& sibling) {
            return sibling && sibling.get() == node.get();
        });
    if (iterator == siblings.end()) {
        throw std::runtime_error(
            "predicate search result is not attached to its parent child list");
    }

    _current = parent;
    _selectedIndex = static_cast<std::size_t>(std::distance(siblings.begin(), iterator));
    _firstVisibleIndex = _selectedIndex;
    _rootSelected = false;
    ensureSelectionVisible();
    ensureVisiblePage();
}

void TerminalModel::ensureMatchSelectionVisible() {
    const std::size_t visibleRows = matchViewportRows();
    if (_searchResultIndex < _firstVisibleMatchIndex) {
        _firstVisibleMatchIndex = _searchResultIndex;
        return;
    }

    const std::size_t lastVisibleExclusive = _firstVisibleMatchIndex >
            std::numeric_limits<std::size_t>::max() - visibleRows
        ? std::numeric_limits<std::size_t>::max()
        : _firstVisibleMatchIndex + visibleRows;
    if (_searchResultIndex >= lastVisibleExclusive) {
        _firstVisibleMatchIndex = _searchResultIndex - visibleRows + 1;
    }
}

void TerminalModel::search(const std::string& predicate, const bool outward) {
    if (_viewMode != ViewMode::Node) {
        _statusMessage = "Search is available from the node view only.";
        return;
    }

    const std::shared_ptr<Node> anchor = selectedNode() ? selectedNode() : _current;
    if (!anchor) {
        _statusMessage = "Search cannot start without a selected node.";
        return;
    }

    const std::string direction = outward ? "\\" : "/";
    const std::string normalizedPredicate =
        !predicate.empty() && predicate.front() == direction.front()
        ? predicate.substr(1)
        : predicate;
    const std::string expression = direction + normalizedPredicate;

    if (normalizedPredicate.empty()) {
        _searchResults.clear();
        _searchExpression = expression;
        _viewMode = ViewMode::Matches;
        _searchResultIndex = 0;
        _firstVisibleMatchIndex = 0;
        _statusMessage = "Search " + expression + ": 0 matches.";
        return;
    }

    try {
        auto results = anchor->pick().allByPredicate(expression);
        _searchResults = std::move(results);
    } catch (const std::exception& error) {
        _statusMessage = "Search error: " + std::string(error.what());
        return;
    }

    _viewMode = ViewMode::Matches;
    _searchExpression = expression;
    _searchResultIndex = 0;
    _firstVisibleMatchIndex = 0;
    ensureMatchSelectionVisible();
    _statusMessage = "Search " + expression + ": " +
        std::to_string(_searchResults.size()) + " match" +
        (_searchResults.size() == 1 ? "" : "es") + ".";
}

bool TerminalModel::canEditPayload() const {
    return _viewMode == ViewMode::Payload && static_cast<bool>(_payloadNode);
}

void TerminalModel::editPayload(const std::string& expression) {
    if (_viewMode != ViewMode::Payload || !_payloadNode) {
        _statusMessage = "Open a payload view before editing a payload.";
        return;
    }

    try {
        TemporaryDirectory temporaryDirectory;
        const auto expressionPath = temporaryDirectory.path() / "expression.txt";
        const auto resultPath = temporaryDirectory.path() / "result.npy";
        const auto logPath = temporaryDirectory.path() / "error.txt";
        {
            std::ofstream prompt(expressionPath, std::ios::binary | std::ios::trunc);
            if (!prompt) {
                throw std::runtime_error("could not write the payload expression prompt");
            }
            prompt.write(expression.data(), static_cast<std::streamsize>(expression.size()));
        }

        const char* configuredPython = std::getenv("NODER_PYTHON_EXECUTABLE");
#ifdef _WIN32
        const std::string python = configuredPython && *configuredPython
            ? configuredPython
            : "python";
#else
        const std::string python = configuredPython && *configuredPython
            ? configuredPython
            : "python3";
#endif
        const int exitCode = runPayloadEvaluator(
            python, expressionPath, _reader->filename(), _payloadNode->path(),
            resultPath, std::string(1, _reader->order()), logPath);
        if (exitCode != 0) {
            std::string detail = readFileText(logPath);
            if (detail.empty()) {
                detail = "Python payload evaluator exited with code " +
                    std::to_string(exitCode);
            }
            _statusMessage = detail;
            return;
        }

        const std::shared_ptr<Node> selected = _payloadNode;
        const std::shared_ptr<Data> previousData = selected->dataPtr();
        const std::shared_ptr<Data> editedData = readPythonPayloadResult(resultPath);
        selected->setData(editedData);
        try {
            selected->saveThisNodeOnly(_reader->filename());
        } catch (...) {
            selected->setData(previousData);
            throw;
        }

        rememberPayload(selected, selected->data());
        _payloadLines = detailedPayloadLines(selected->data());
        _payloadScrollOffset = 0;
        _statusMessage = "Payload updated in-place.";
    } catch (const std::exception& error) {
        _statusMessage = "Payload edit failed: " + std::string(error.what());
    }
}

std::string TerminalModel::payloadMarker(const std::shared_ptr<Node>& node) const {
    if (!node || !node->hasData()) {
        return "";
    }

    const auto iterator = _payloadDisplays.find(node.get());
    if (iterator == _payloadDisplays.end()) {
        return "  \033[3m[press d to show payload]\033[0m";
    }

    if (iterator->second.state == PayloadDisplay::State::TooBig) {
        return "  \033[33m[too big size to show]\033[0m";
    }

    if (iterator->second.state == PayloadDisplay::State::Summary) {
        return "  \033[36m[" + iterator->second.markerText + "]\033[0m";
    }

    if (!iterator->second.markerText.empty()) {
        return "  \033[36m" + iterator->second.markerText + "\033[0m";
    }

    // Numerical arrays are rendered in the status area below the navigation.
    // Lightweight values such as strings remain visible beside their node.
    return "";
}

void TerminalModel::moveSelection(const long long delta) {
    ensureVisiblePage();
    if (_current->loadedChildren().empty()) {
        _selectedIndex = 0;
        return;
    }

    if (_rootSelected) {
        if (delta <= 0) {
            return;
        }
        const std::size_t amount = static_cast<std::size_t>(delta);
        _rootSelected = false;
        _selectedIndex = std::min(
            amount == 0 ? 0 : amount - 1,
            _current->loadedChildren().size() - 1);
        ensureSelectionVisible();
        return;
    }

    if (delta < 0) {
        const std::size_t amount = static_cast<std::size_t>(-delta);
        _selectedIndex = amount > _selectedIndex ? 0 : _selectedIndex - amount;
        ensureSelectionVisible();
        return;
    }

    const std::size_t amount = static_cast<std::size_t>(delta);
    const std::size_t target = _selectedIndex > std::numeric_limits<std::size_t>::max() - amount
        ? std::numeric_limits<std::size_t>::max()
        : _selectedIndex + amount;
    if (target < _current->loadedChildren().size()) {
        _selectedIndex = target;
        ensureSelectionVisible();
        return;
    }

    if (_current->childrenLoadState() != ChildrenLoadState::Complete) {
        const std::size_t requested = target == std::numeric_limits<std::size_t>::max()
            ? target
            : target + 1;
        _current->ensureChildrenLoaded(requested);
        if (target < _current->loadedChildren().size()) {
            _selectedIndex = target;
            ensureSelectionVisible();
            return;
        }
    }

    _selectedIndex = _current->loadedChildren().size() - 1;
    ensureSelectionVisible();
}

void TerminalModel::moveMatchSelection(const long long delta) {
    if (_searchResults.empty()) {
        _searchResultIndex = 0;
        _firstVisibleMatchIndex = 0;
        return;
    }

    if (delta < 0) {
        const std::size_t amount = static_cast<std::size_t>(-delta);
        _searchResultIndex = amount > _searchResultIndex
            ? 0
            : _searchResultIndex - amount;
        ensureMatchSelectionVisible();
        return;
    }

    const std::size_t amount = static_cast<std::size_t>(delta);
    const std::size_t target = _searchResultIndex >
            std::numeric_limits<std::size_t>::max() - amount
        ? std::numeric_limits<std::size_t>::max()
        : _searchResultIndex + amount;
    _searchResultIndex = std::min(target, _searchResults.size() - 1);
    ensureMatchSelectionVisible();
}

void TerminalModel::selectFirstChild() {
    _rootSelected = false;
    _selectedIndex = 0;
    _firstVisibleIndex = 0;
    ensureVisiblePage();
}

void TerminalModel::selectLastChild() {
    _current->ensureChildrenLoaded();
    if (_current->loadedChildren().empty()) {
        _rootSelected = false;
        _selectedIndex = 0;
        _firstVisibleIndex = 0;
        return;
    }
    _rootSelected = false;
    _selectedIndex = _current->loadedChildren().size() - 1;
    ensureSelectionVisible();
}

void TerminalModel::selectFirstMatch() {
    _searchResultIndex = 0;
    _firstVisibleMatchIndex = 0;
    ensureMatchSelectionVisible();
}

void TerminalModel::selectLastMatch() {
    if (_searchResults.empty()) {
        _searchResultIndex = 0;
        _firstVisibleMatchIndex = 0;
        return;
    }
    _searchResultIndex = _searchResults.size() - 1;
    ensureMatchSelectionVisible();
}

void TerminalModel::enterSelectedChildren() {
    if (_rootSelected) {
        _current->ensureChildrenLoaded(_pageSize);
        if (_current->loadedChildren().empty()) {
            _statusMessage = "Root node has no children.";
            return;
        }
        _rootSelected = false;
        _selectedIndex = 0;
        _firstVisibleIndex = 0;
        _statusMessage.clear();
        return;
    }

    const std::shared_ptr<Node> selected = selectedNode();
    if (!selected) {
        _statusMessage = "No child is selected.";
        return;
    }

    selected->ensureChildrenLoaded(_pageSize);
    if (selected->loadedChildren().empty()) {
        _statusMessage = "Node '" + selected->name() + "' has no children.";
        return;
    }

    _current = selected;
    _selectedIndex = 0;
    _firstVisibleIndex = 0;
    _rootSelected = false;
    _statusMessage.clear();
}

void TerminalModel::enterSelectedMatchChildren() {
    const std::shared_ptr<Node> selected = selectedMatch();
    if (!selected) {
        _statusMessage = "No match is selected.";
        return;
    }

    selected->ensureChildrenLoaded(_pageSize);
    if (selected->loadedChildren().empty()) {
        _statusMessage = "Match '" + selected->name() + "' has no children.";
        return;
    }

    _current = selected;
    _selectedIndex = 0;
    _firstVisibleIndex = 0;
    _rootSelected = selected.get() == _reader->root().get();
    _viewMode = ViewMode::Node;
    _statusMessage.clear();
}

void TerminalModel::leaveToParent() {
    if (!_current) {
        return;
    }
    if (_current.get() == _reader->root().get() && !_rootSelected) {
        selectRoot();
        return;
    }
    const std::shared_ptr<Node> parent = _current->parent().lock();
    if (!parent) {
        _statusMessage = "Already at the root node.";
        return;
    }

    parent->ensureChildrenLoaded();
    const auto& siblings = parent->loadedChildren();
    const auto iterator = std::find_if(
        siblings.begin(),
        siblings.end(),
        [this](const std::shared_ptr<Node>& sibling) {
            return sibling && sibling.get() == _current.get();
        });

    _current = parent;
    _selectedIndex = iterator == siblings.end()
        ? 0
        : static_cast<std::size_t>(std::distance(siblings.begin(), iterator));
    _firstVisibleIndex = _selectedIndex;
    ensureSelectionVisible();
    _statusMessage.clear();
}

void TerminalModel::leaveMatchesToParent() {
    const std::shared_ptr<Node> selected = selectedMatch();
    if (!selected) {
        _statusMessage = "No match is selected.";
        return;
    }

    if (!selected->parent().lock()) {
        _statusMessage = "Match '" + selected->name() + "' has no parent node.";
        return;
    }

    _viewMode = ViewMode::Node;
    selectNodeInNodeView(selected);
    _statusMessage.clear();
}

void TerminalModel::showMatches() {
    if (_searchExpression.empty()) {
        _statusMessage = "No active search results.";
        return;
    }

    _viewMode = ViewMode::Matches;
    _payloadNode.reset();
    _payloadLines.clear();
    _payloadScrollOffset = 0;
    ensureMatchSelectionVisible();
    _statusMessage = "Search " + _searchExpression + ": " +
        std::to_string(_searchResults.size()) + " match" +
        (_searchResults.size() == 1 ? "" : "es") + ".";
}

void TerminalModel::rememberPayload(const std::shared_ptr<Node>& selected, const Data& data) {
    const auto* array = dynamic_cast<const Array*>(&data);
    const bool numerical = array != nullptr && isNumericalArray(data);

    std::string payloadText;
    PayloadDisplay display{
        PayloadDisplay::State::Displayed,
        "",
        ""};

    if (data.hasString()) {
        const std::string stringValue = data.extractString();
        const std::string marker = stringMarkerText(stringValue, _maxPayloadChars);
        const bool summarized = stringValue.size() > _maxPayloadChars;
        payloadText = summarized ? marker : stringValue;
        display.state = summarized ? PayloadDisplay::State::Summary : PayloadDisplay::State::Displayed;
        display.text = payloadText;
        display.markerText = marker;
    } else if (numerical) {
        const std::string summary = numericalSummary(*array);
        payloadText = summary;
        display.text = payloadText;
        if (data.size() <= 9) {
            display.state = PayloadDisplay::State::Displayed;
            display.markerText = numericalArrayText(*array);
        } else {
            display.state = PayloadDisplay::State::Summary;
            display.markerText = summary;
        }
    } else if (!numerical && data.size() > _payloadElementLimit) {
        display.state = PayloadDisplay::State::TooBig;
        _payloadDisplays[selected.get()] = display;
        _statusMessage = "Payload has " + std::to_string(data.size()) +
            " elements; display limit is " + std::to_string(_payloadElementLimit) + ".";
        return;
    } else {
        payloadText = data.shortInfo();
        display.text = payloadText;
        display.markerText = data.isScalar()
            ? payloadText
            : "";
    }

    _payloadDisplays[selected.get()] = display;
    std::ostringstream stream;
    stream << selected->path() << " : " << selected->type() << "\n";
    stream << "payload (" << data.size() << " element(s), " << data.dtype()
           << ", shape=" << shapeText(data) << "): ";
    stream << payloadText;
    _statusMessage = stream.str();
}

void TerminalModel::enterSelectedPayload() {
    const std::shared_ptr<Node> selected = selectedNode();
    if (!selected) {
        _statusMessage = "No child is selected.";
        return;
    }
    if (!selected->hasData()) {
        _statusMessage = "Node '" + selected->name() + "' has no payload.";
        return;
    }

    rememberPayload(selected, selected->data());
}

void TerminalModel::enterSelectedPayloadView() {
    const std::shared_ptr<Node> selected = selectedNode();
    if (!selected) {
        _statusMessage = "No child is selected.";
        return;
    }
    if (!selected->hasData()) {
        _statusMessage = "Node '" + selected->name() + "' has no payload.";
        return;
    }

    const Data& data = selected->data();
    rememberPayload(selected, data);
    _payloadNode = selected;
    _payloadLines = detailedPayloadLines(data);
    _payloadScrollOffset = 0;
    _viewMode = ViewMode::Payload;
    _statusMessage.clear();
}

void TerminalModel::enterSelectedMatchPayload() {
    enterSelectedPayloadView();
}

void TerminalModel::saveSelectedPayload() {
    const std::shared_ptr<Node> selected = selectedNode();
    if (!selected) {
        _statusMessage = "No node is selected.";
        return;
    }
    if (!selected->hasData()) {
        _statusMessage = "Node '" + selected->name() + "' has no payload.";
        return;
    }

    const std::string fileName = payloadFileName(selected->name());
    std::ofstream file(fileName, std::ios::out | std::ios::trunc);
    if (!file) {
        _statusMessage = "Could not save payload to " + fileName + ".";
        return;
    }

    const std::vector<std::string> lines = detailedPayloadLines(selected->data());
    for (const std::string& line : lines) {
        file << line << '\n';
    }
    if (!file) {
        _statusMessage = "Could not save payload to " + fileName + ".";
        return;
    }
    _statusMessage = "Saved payload to " + fileName + ".";
}

void TerminalModel::scrollPayload(const long long delta) {
    if (_viewMode != ViewMode::Payload || _payloadLines.empty()) {
        _payloadScrollOffset = 0;
        return;
    }

    const std::size_t visibleRows = payloadViewportRows();
    const std::size_t maximumOffset = _payloadLines.size() > visibleRows
        ? _payloadLines.size() - visibleRows
        : 0;
    if (delta < 0) {
        const std::size_t amount = static_cast<std::size_t>(-delta);
        _payloadScrollOffset = amount > _payloadScrollOffset
            ? 0
            : _payloadScrollOffset - amount;
        return;
    }

    const std::size_t amount = static_cast<std::size_t>(delta);
    if (amount >= maximumOffset || _payloadScrollOffset >= maximumOffset - amount) {
        _payloadScrollOffset = maximumOffset;
    } else {
        _payloadScrollOffset += amount;
    }
}

void TerminalModel::leavePayloadView() {
    const std::shared_ptr<Node> payloadNode = _payloadNode;
    _viewMode = ViewMode::Node;
    _payloadNode.reset();
    _payloadLines.clear();
    _payloadScrollOffset = 0;
    if (payloadNode) {
        selectNodeInNodeView(payloadNode);
    }
    _statusMessage.clear();
}

bool TerminalModel::handle(const Key key) {
    if (key == Key::Quit) {
        return false;
    }

    if (key == Key::SelectRoot) {
        selectRoot();
        return true;
    }

    if (key == Key::ShowMatches) {
        showMatches();
        return true;
    }

    if (key == Key::SavePayload) {
        saveSelectedPayload();
        return true;
    }

    switch (_viewMode) {
        case ViewMode::Payload:
            switch (key) {
                case Key::Up:
                    scrollPayload(-1);
                    break;
                case Key::Down:
                    scrollPayload(1);
                    break;
                case Key::PageUp:
                    scrollPayload(-static_cast<long long>(payloadViewportRows()));
                    break;
                case Key::PageDown:
                    scrollPayload(static_cast<long long>(payloadViewportRows()));
                    break;
                case Key::Home:
                    _payloadScrollOffset = 0;
                    break;
                case Key::End:
                    scrollPayload(std::numeric_limits<long long>::max());
                    break;
                case Key::Left:
                case Key::Right:
                case Key::ShiftEnter:
                case Key::SearchInward:
                case Key::SearchOutward:
                case Key::Escape:
                    leavePayloadView();
                    break;
                case Key::Delete:
                case Key::EditPayload:
                case Key::Enter:
                case Key::SavePayload:
                case Key::Unknown:
                case Key::ShowMatches:
                case Key::Quit:
                    break;
            }
            break;

        case ViewMode::Matches:
            switch (key) {
                case Key::Up:
                    moveMatchSelection(-1);
                    break;
                case Key::Down:
                    moveMatchSelection(1);
                    break;
                case Key::PageUp:
                    moveMatchSelection(-static_cast<long long>(matchViewportRows()));
                    break;
                case Key::PageDown:
                    moveMatchSelection(static_cast<long long>(matchViewportRows()));
                    break;
                case Key::Home:
                    selectFirstMatch();
                    break;
                case Key::End:
                    selectLastMatch();
                    break;
                case Key::Left:
                    leaveMatchesToParent();
                    break;
                case Key::Right:
                    enterSelectedMatchChildren();
                    break;
                case Key::ShowSummary:
                    enterSelectedPayload();
                    break;
                case Key::ShowDetails:
                    enterSelectedMatchPayload();
                    break;
                case Key::Enter:
                case Key::ShiftEnter:
                case Key::SavePayload:
                case Key::Delete:
                case Key::EditPayload:
                    break;
                case Key::Escape:
                    _viewMode = ViewMode::Node;
                    _statusMessage.clear();
                    break;
                case Key::SearchInward:
                case Key::SearchOutward:
                case Key::Unknown:
                case Key::ShowMatches:
                case Key::Quit:
                    break;
            }
            break;

        case ViewMode::Node:
            switch (key) {
                case Key::Up:
                    moveSelection(-1);
                    break;
                case Key::Down:
                    moveSelection(1);
                    break;
                case Key::PageUp:
                    moveSelection(-static_cast<long long>(_pageSize));
                    break;
                case Key::PageDown:
                    moveSelection(static_cast<long long>(_pageSize));
                    break;
                case Key::Home:
                    selectFirstChild();
                    break;
                case Key::End:
                    selectLastChild();
                    break;
                case Key::Left:
                    leaveToParent();
                    break;
                case Key::Right:
                    enterSelectedChildren();
                    break;
                case Key::ShowSummary:
                    enterSelectedPayload();
                    break;
                case Key::ShowDetails:
                    enterSelectedPayloadView();
                    break;
                case Key::SavePayload:
                case Key::Delete:
                case Key::EditPayload:
                case Key::SearchInward:
                case Key::SearchOutward:
                case Key::Escape:
                case Key::Unknown:
                case Key::ShowMatches:
                case Key::Quit:
                    break;
            }
            break;
    }
    return true;
}

void TerminalModel::renderNodes(std::ostream& output) const {
    const std::size_t visibleRows = childViewportRows();
    const std::size_t remaining = std::numeric_limits<std::size_t>::max() - _firstVisibleIndex;
    _current->ensureChildrenLoaded(
        visibleRows > remaining ? std::numeric_limits<std::size_t>::max() : _firstVisibleIndex + visibleRows);

    output << "\x1b[2J\x1b[H";
    output << "CGNSviz from package NODER v" << NODER_VERSION << " (c) ONERA\n";
    output << "file:  " << _reader->filename() << "\n";
    output << "path: " << (_rootSelected ? "\033[1;7m" : "\033[1;4m")
           << _current->path() << "\033[0m\n";
    output << "type: " << _current->type()
           << "    children: " << loadStateName(_current->childrenLoadState())
           << "    loaded: " << _current->loadedChildren().size();
    if (_rootSelected) {
        output << "    selection: root";
    }
    output << "\n\n";

    const auto& children = _current->loadedChildren();
    if (children.empty()) {
        output << "(no children)\n";
    } else {
        const std::size_t first = std::min(_firstVisibleIndex, children.size());
        const std::size_t last = std::min(children.size(), first + visibleRows);
        if (first > 0) {
            output << "  ... previous children hidden ...\n";
        }
        for (std::size_t index = first; index < last; ++index) {
            const auto& child = children[index];
            if (!child) {
                continue;
            }
            output << (!_rootSelected && index == _selectedIndex ? " >\033[7m" : "  ");
            output << child->name() << "  \033[90m" << child->type() << "\033[0m"
                   << payloadMarker(child) << "\n";
        }
        if (last < children.size() || _current->childrenLoadState() != ChildrenLoadState::Complete) {
            output << "  ... more children available ...\n";
        }
    }

    output << "\n[Up/Down] select  [PgUp/PgDn] page  [Home/End] first/last  [Ctrl+Home] root  [Right] open  [Left] parent\n"
               "[d] summary  [Shift+d] details  [/] descendant search  [\\] ancestor search\n"
               "[Ctrl+S] save payload  [m] matches  [q] quit\n";
    if (!_statusMessage.empty()) {
        output << "\n" << _statusMessage << "\n";
    }
    renderMalformedWarning(output);
}

void TerminalModel::renderMatches(std::ostream& output) const {
    const std::size_t visibleRows = matchViewportRows();

    output << "\x1b[2J\x1b[H";
    output << "cgnsviz  matches view\n";
    output << "search: " << _searchExpression << "    matches: "
           << _searchResults.size() << "\n\n";

    if (_searchResults.empty()) {
        output << "(no matches)\n";
    } else {
        const std::size_t first = std::min(_firstVisibleMatchIndex, _searchResults.size());
        const std::size_t last = std::min(_searchResults.size(), first + visibleRows);
        if (first > 0) {
            output << "  ... previous matches hidden ...\n";
        }
        for (std::size_t index = first; index < last; ++index) {
            const auto& match = _searchResults[index];
            if (!match) {
                continue;
            }
            const std::string path = compactMatchPath(match->path());
            output << (index == _searchResultIndex ? " >\033[7m" : "  ");
            output << match->name() << "  \033[90m" << match->type() << "\033[0m";
            if (!path.empty()) {
                output << "  \033[2m" << path << "\033[0m";
            }
            output << payloadMarker(match) << "\n";
        }
        if (last < _searchResults.size()) {
            output << "  ... more matches available ...\n";
        }
    }

    output << "\n[Up/Down] select  [PgUp/PgDn] page  [Home/End] first/last  [Right] children  [Left] parent\n"
               "[d] summary  [Shift+d] details  [Escape] node view  [Ctrl+S] save payload  [m] matches  [q] quit\n";
    if (!_statusMessage.empty()) {
        output << "\n" << _statusMessage << "\n";
    }
    renderMalformedWarning(output);
}

void TerminalModel::renderPayload(std::ostream& output) const {
    const bool compact = _viewportRows < 7;
    const std::string path = _payloadNode ? _payloadNode->path() : "";
    std::string metadata;
    if (_payloadNode) {
        const Data& data = _payloadNode->data();
        std::ostringstream metadataStream;
        metadataStream << "type: " << _payloadNode->type()
                       << "    dtype: " << data.dtype()
                       << "    elements: " << data.size()
                       << "    shape: " << shapeText(data);
        metadata = metadataStream.str();
    } else {
        metadata = "type: unknown";
    }
    if (compact && !_statusMessage.empty()) {
        metadata += "  [" + _statusMessage + "]";
    }

    output << "\x1b[2J\x1b[H";
    if (!compact) {
        output << "cgnsviz  payload view\n";
        output << "path: " << path << "\n";
        output << metadata << "\n\n";
    } else if (_viewportRows >= 4) {
        output << "cgnsviz  payload view\n";
        output << "path: " << path << "\n";
        output << metadata << "\n";
    } else if (_viewportRows == 3) {
        output << "cgnsviz  payload view  path: " << path << "\n";
        output << metadata << "\n";
    } else if (_viewportRows == 2) {
        output << metadata << "\n";
    }

    const std::size_t first = std::min(_payloadScrollOffset, _payloadLines.size());
    const std::size_t last = std::min(_payloadLines.size(), first + payloadViewportRows());
    if (_viewportRows == 1) {
        output << metadata;
        if (first < _payloadLines.size()) {
            output << "  " << _payloadLines[first];
        }
        output << "\n";
        renderMalformedWarning(output);
        return;
    }
    if (_payloadLines.empty()) {
        output << "(empty payload)\n";
    } else {
        for (std::size_t index = first; index < last; ++index) {
            output << _payloadLines[index] << "\n";
        }
    }

    if (!compact) {
        output << "\n[Up/Down] scroll  [PgUp/PgDn] page  [Home/End] first/last  [e] edit payload  [Escape] back to node view  [Ctrl+S] save payload  [m] matches  [q] quit";
        if (!_statusMessage.empty()) {
            output << "  " << _statusMessage;
        }
        output << "\n";
    }
    renderMalformedWarning(output);
}

void TerminalModel::render(std::ostream& output) const {
    switch (_viewMode) {
        case ViewMode::Node:
            renderNodes(output);
            break;
        case ViewMode::Matches:
            renderMatches(output);
            break;
        case ViewMode::Payload:
            renderPayload(output);
            break;
    }
}

namespace {

std::size_t terminalRows() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return static_cast<std::size_t>(info.srWindow.Bottom - info.srWindow.Top + 1);
    }
#else
    winsize size{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_row > 0) {
        return static_cast<std::size_t>(size.ws_row);
    }
#endif
    return 24;
}

#ifndef _WIN32
class RawTerminal {
public:
    RawTerminal() {
        if (!isatty(STDIN_FILENO)) {
            throw std::runtime_error("cgnsviz requires an interactive terminal");
        }
        if (tcgetattr(STDIN_FILENO, &_original) != 0) {
            throw std::runtime_error("cgnsviz: cannot read terminal settings");
        }
        termios raw = _original;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_iflag &= static_cast<tcflag_t>(~(IXON | IXOFF));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            throw std::runtime_error("cgnsviz: cannot enable raw terminal mode");
        }
    }

    ~RawTerminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &_original);
    }

private:
    termios _original{};
};

class ModifiedKeyMode {
public:
    explicit ModifiedKeyMode(std::ostream& output) : _output(output) {
        // Ask xterm-compatible terminals to preserve Shift+Enter as a distinct
        // CSI sequence. Terminals without this feature safely ignore it.
        _output << "\x1b[>4;2m";
        _output.flush();
    }

    ~ModifiedKeyMode() {
        _output << "\x1b[>4;0m";
        _output.flush();
    }

private:
    std::ostream& _output;
};
#endif

class AlternateScreen {
public:
    explicit AlternateScreen(std::ostream& output) : _output(output) {
        _output << "\x1b[?1049h\x1b[2J\x1b[H";
        _output.flush();
    }

    ~AlternateScreen() {
        _output << "\x1b[?1049l";
        _output.flush();
    }

    AlternateScreen(const AlternateScreen&) = delete;
    AlternateScreen& operator=(const AlternateScreen&) = delete;

private:
    std::ostream& _output;
};

int readByte() {
#ifdef _WIN32
    return _getch();
#else
    unsigned char value = 0;
    const ssize_t readCount = ::read(STDIN_FILENO, &value, 1);
    return readCount == 1 ? static_cast<int>(value) : -1;
#endif
}

int readByteIfAvailable() {
#ifdef _WIN32
    return _kbhit() != 0 ? _getch() : -1;
#else
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);
    timeval timeout{};
    timeout.tv_usec = 50000;
    const int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
    return ready > 0 && FD_ISSET(STDIN_FILENO, &readSet) ? readByte() : -1;
#endif
}

Key parseCsiSequence(const std::string& sequence) {
    if (sequence == "A") return Key::Up;
    if (sequence == "B") return Key::Down;
    if (sequence == "C") return Key::Right;
    if (sequence == "D") return Key::Left;
    if (sequence == "5~") return Key::PageUp;
    if (sequence == "6~") return Key::PageDown;
    if (sequence == "3~") return Key::Delete;
    if (sequence == "1;5H" || sequence == "1;5~" || sequence == "7;5~") {
        return Key::SelectRoot;
    }
    if (sequence == "H" || sequence == "1~" || sequence == "7~") return Key::Home;
    if (sequence == "F" || sequence == "4~" || sequence == "8~") return Key::End;
    if (sequence == "13;2u" || sequence == "27;2;13~") return Key::ShiftEnter;
    return Key::Unknown;
}

template <typename ReadNext>
Key readCsiSequence(ReadNext&& readNext) {
    const int first = readNext();
    if (first < 0) {
        return Key::Unknown;
    }

    std::string sequence(1, static_cast<char>(first));
    if (std::isalpha(static_cast<unsigned char>(first)) != 0) {
        return parseCsiSequence(sequence);
    }

    while (true) {
        const int value = readNext();
        if (value < 0) {
            return Key::Unknown;
        }
        sequence.push_back(static_cast<char>(value));
        if (value == '~' || std::isalpha(static_cast<unsigned char>(value)) != 0) {
            return parseCsiSequence(sequence);
        }
    }
}

Key readKey() {
    const int first = readByte();
    if (first < 0) {
        return Key::Quit;
    }

#ifdef _WIN32
    if (first == 0 || first == 224) {
        switch (readByte()) {
            case 72: return Key::Up;
            case 80: return Key::Down;
            case 75: return Key::Left;
            case 77: return Key::Right;
            case 73: return Key::PageUp;
            case 81: return Key::PageDown;
            case 71: return Key::Home;
            case 79: return Key::End;
            case 83: return Key::Delete;
            case 119: return Key::SelectRoot;
            default: return Key::Unknown;
        }
    }
#endif

    if (first == 27) {
        const int second = readByteIfAvailable();
        if (second != '[') {
            return Key::Escape;
        }
        return readCsiSequence([]() { return readByte(); });
    }

    if (first == '\r' || first == '\n') {
#ifdef _WIN32
        if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) {
            return Key::ShiftEnter;
        }
#endif
        return Key::Enter;
    }
    if (first == 'q' || first == 'Q') return Key::Quit;
    if (first == 19) return Key::SavePayload;
    if (first == 'd') return Key::ShowSummary;
    if (first == 'D') return Key::ShowDetails;
    if (first == 'e' || first == 'E') return Key::EditPayload;
    if (first == '/') return Key::SearchInward;
    if (first == '\\') return Key::SearchOutward;
    if (first == 'm') return Key::ShowMatches;
    if (first == 27) return Key::Escape;
    return Key::Unknown;
}

std::string readInteractiveLine(const std::string& prompt, bool& accepted) {
    accepted = false;
    std::string line;
    std::size_t cursor = 0;
    const auto redraw = [&]() {
        std::cout << "\r" << prompt << line << "\033[K";
        if (cursor < line.size()) {
            std::cout << "\033[" << (line.size() - cursor) << "D";
        }
        std::cout.flush();
    };
    std::cout << prompt << std::flush;
    while (true) {
        const int value = readByte();
        if (value < 0) {
            return line;
        }
#ifdef _WIN32
        if (value == 0 || value == 224) {
            Key key = Key::Unknown;
            switch (readByte()) {
                case 75: key = Key::Left; break;
                case 77: key = Key::Right; break;
                case 71: key = Key::Home; break;
                case 79: key = Key::End; break;
                case 83: key = Key::Delete; break;
                default: break;
            }
            if (key == Key::Left && cursor > 0) {
                --cursor;
                redraw();
            } else if (key == Key::Right && cursor < line.size()) {
                ++cursor;
                redraw();
            } else if (key == Key::Home) {
                cursor = 0;
                redraw();
            } else if (key == Key::End) {
                cursor = line.size();
                redraw();
            } else if (key == Key::Delete && cursor < line.size()) {
                line.erase(cursor, 1);
                redraw();
            }
            continue;
        }
#endif
        if (value == '\r' || value == '\n') {
            std::cout << "\n" << std::flush;
            accepted = true;
            return line;
        }
        if (value == 27) {
            const int second = readByteIfAvailable();
            if (second != '[') {
                std::cout << "\n" << std::flush;
                return line;
            }
            const Key key = readCsiSequence([]() { return readByte(); });
            if (key == Key::Left && cursor > 0) {
                --cursor;
                redraw();
            } else if (key == Key::Right && cursor < line.size()) {
                ++cursor;
                redraw();
            } else if (key == Key::Home) {
                cursor = 0;
                redraw();
            } else if (key == Key::End) {
                cursor = line.size();
                redraw();
            } else if (key == Key::Delete && cursor < line.size()) {
                line.erase(cursor, 1);
                redraw();
            }
            continue;
        }
        if (value == 8 || value == 127) {
            if (cursor > 0) {
                line.erase(cursor - 1, 1);
                --cursor;
                redraw();
            }
            continue;
        }
        if (value >= 32 && value != 127) {
            line.insert(line.begin() + static_cast<std::ptrdiff_t>(cursor),
                        static_cast<char>(value));
            ++cursor;
            redraw();
        }
    }
}

} // namespace

int runTerminal(TerminalModel& model, std::istream& input, std::ostream& output) {
    model.render(output);
    output.flush();
    while (input.good()) {
        char value = 0;
        if (!input.get(value)) {
            break;
        }

        Key key = Key::Unknown;
        if (value == '/' || value == '\\') {
            std::string predicate;
            char next = 0;
            while (input.get(next) && next != '\n' && next != '\r') {
                predicate.push_back(next);
            }
            model.search(predicate, value == '\\');
            model.render(output);
            output.flush();
            continue;
        } else if (value == 'e' && model.canEditPayload()) {
            std::string expression;
            std::getline(input, expression);
            if (!expression.empty() && expression.back() == '\r') {
                expression.pop_back();
            }
            model.editPayload(expression);
            model.render(output);
            output.flush();
            continue;
        } else if (value == 'm') {
            model.handle(Key::ShowMatches);
            model.render(output);
            output.flush();
            continue;
        } else if (value == '\x1b') {
            if (input.peek() == '[') {
                input.get();
                key = readCsiSequence([&input]() {
                    char next = 0;
                    return input.get(next) ? static_cast<int>(next) : -1;
                });
            } else {
                key = Key::Escape;
            }
        } else {
            switch (value) {
                case 'k': key = Key::Up; break;
                case 'j': key = Key::Down; break;
                case 'h': key = Key::Left; break;
                case 'l': key = Key::Right; break;
                case 'd': key = Key::ShowSummary; break;
                case 'D': key = Key::ShowDetails; break;
                case 'e': key = Key::EditPayload; break;
                case '\x13': key = Key::SavePayload; break;
                case 'q': key = Key::Quit; break;
                case '\n': case '\r': key = Key::Enter; break;
                default: break;
            }
        }
        if (!model.handle(key)) {
            return 0;
        }
        model.render(output);
        output.flush();
    }
    return 0;
}

int runInteractiveTerminal(TerminalModel& model) {
#ifndef _WIN32
    RawTerminal rawTerminal;
    ModifiedKeyMode modifiedKeyMode(std::cout);
#endif
    AlternateScreen alternateScreen(std::cout);
    model.setViewportRows(terminalRows());
    model.render(std::cout);
    std::cout.flush();
    while (true) {
        const Key key = readKey();
        if (key == Key::SearchInward || key == Key::SearchOutward) {
            bool accepted = false;
            const std::string predicate = readInteractiveLine(
                key == Key::SearchOutward ? "\\" : "/", accepted);
            if (accepted) {
                model.search(predicate, key == Key::SearchOutward);
            }
        } else if (key == Key::EditPayload && model.canEditPayload()) {
            bool accepted = false;
            const std::string expression = readInteractiveLine("payload> ", accepted);
            if (accepted) {
                model.editPayload(expression);
            }
        } else if (!model.handle(key)) {
            return 0;
        }
        model.setViewportRows(terminalRows());
        model.render(std::cout);
        std::cout.flush();
    }
}

} // namespace cgnsviz::terminal
