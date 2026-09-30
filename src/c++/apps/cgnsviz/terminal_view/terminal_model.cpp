#include "apps/cgnsviz/terminal_view/terminal_model.hpp"

#include "array/array.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"
#endif

#include <cctype>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <cerrno>
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
        return "min=nan, max=nan, mean=nan, median=nan";
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
    stream << std::setprecision(15);
    stream << "min=" << *minIterator
           << ", max=" << *maxIterator
           << ", mean=" << mean
           << ", median=" << median;
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
      _rootSelected(false),
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
      _searchExpression() {

    if (!_reader) {
        throw std::invalid_argument("TerminalModel: reader cannot be null");
    }
    _current = _reader->root();
    if (!_current) {
        throw std::runtime_error("TerminalModel: lazy reader returned a null root");
    }
    ensureVisiblePage();
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
    constexpr std::size_t reservedRows = 6;
    return _viewportRows > reservedRows ? _viewportRows - reservedRows : 1;
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

std::string TerminalModel::payloadMarker(const std::shared_ptr<Node>& node) const {
    if (!node || !node->hasData()) {
        return "";
    }

    const auto iterator = _payloadDisplays.find(node.get());
    if (iterator == _payloadDisplays.end()) {
        return "  \033[3m[press Enter to show payload]\033[0m";
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
                case Key::Enter:
                    leavePayloadView();
                    break;
                case Key::Left:
                case Key::Right:
                case Key::ShiftEnter:
                case Key::SearchInward:
                case Key::SearchOutward:
                case Key::Escape:
                    leavePayloadView();
                    break;
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
                case Key::Enter:
                    enterSelectedPayload();
                    break;
                case Key::ShiftEnter:
                    enterSelectedMatchPayload();
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
                case Key::Enter:
                    enterSelectedPayload();
                    break;
                case Key::ShiftEnter:
                    enterSelectedPayloadView();
                    break;
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
               "[Enter] summary  [Shift+Enter] details  [/] descendant search  [\\] ancestor search\n"
               "[m] matches  [q] quit\n";
    if (!_statusMessage.empty()) {
        output << "\n" << _statusMessage << "\n";
    }
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
               "[Enter] summary  [Shift+Enter] details  [Escape] node view  [m] matches  [q] quit\n";
    if (!_statusMessage.empty()) {
        output << "\n" << _statusMessage << "\n";
    }
}

void TerminalModel::renderPayload(std::ostream& output) const {
    output << "\x1b[2J\x1b[H";
    output << "cgnsviz  payload view\n";
    output << "path: " << (_payloadNode ? _payloadNode->path() : "") << "\n";
    if (_payloadNode) {
        const Data& data = _payloadNode->data();
        output << "type: " << _payloadNode->type()
               << "    dtype: " << data.dtype()
               << "    elements: " << data.size()
               << "    shape: " << shapeText(data) << "\n\n";
    } else {
        output << "\n";
    }

    const std::size_t first = std::min(_payloadScrollOffset, _payloadLines.size());
    const std::size_t last = std::min(_payloadLines.size(), first + payloadViewportRows());
    if (_payloadLines.empty()) {
        output << "(empty payload)\n";
    } else {
        for (std::size_t index = first; index < last; ++index) {
            output << _payloadLines[index] << "\n";
        }
    }

    output << "\n[Up/Down] scroll  [PgUp/PgDn] page  [Home/End] first/last  [Enter] back to node view  [m] matches  [q] quit\n";
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
    if (first == '/') return Key::SearchInward;
    if (first == '\\') return Key::SearchOutward;
    if (first == 'm') return Key::ShowMatches;
    if (first == 27) return Key::Escape;
    return Key::Unknown;
}

std::string readInteractiveSearchLine(const bool outward, bool& accepted) {
    accepted = false;
    std::string predicate;
    std::cout << (outward ? "\\" : "/") << std::flush;
    while (true) {
        const int value = readByte();
        if (value < 0) {
            return predicate;
        }
        if (value == '\r' || value == '\n') {
            std::cout << "\n" << std::flush;
            accepted = true;
            return predicate;
        }
        if (value == 27) {
            std::cout << "\n" << std::flush;
            return predicate;
        }
        if (value == 8 || value == 127) {
            if (!predicate.empty()) {
                predicate.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (std::isprint(static_cast<unsigned char>(value)) != 0) {
            predicate.push_back(static_cast<char>(value));
            std::cout << static_cast<char>(value) << std::flush;
        }
    }
}

} // namespace

int runTerminal(TerminalModel& model, std::istream& input, std::ostream& output) {
    model.render(output);
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
            continue;
        } else if (value == 'm') {
            model.handle(Key::ShowMatches);
            model.render(output);
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
                case 'q': key = Key::Quit; break;
                case '\n': case '\r': key = Key::Enter; break;
                default: break;
            }
        }
        if (!model.handle(key)) {
            return 0;
        }
        model.render(output);
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
            const std::string predicate = readInteractiveSearchLine(
                key == Key::SearchOutward,
                accepted);
            if (accepted) {
                model.search(predicate, key == Key::SearchOutward);
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
