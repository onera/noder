#include "apps/cgnsviz/terminal_view/terminal_model.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"
#endif

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#else
#include <cerrno>
#include <termios.h>
#include <unistd.h>
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

} // namespace

TerminalModel::TerminalModel(
    std::shared_ptr<io::hdf5::cgns::LazyHdf5Reader> reader,
    const std::size_t pageSize,
    const std::size_t payloadElementLimit)
    : _reader(std::move(reader)),
      _current(nullptr),
      _selectedIndex(0),
      _pageSize(std::max<std::size_t>(1, pageSize)),
      _payloadElementLimit(payloadElementLimit),
      _statusMessage() {

    if (!_reader) {
        throw std::invalid_argument("TerminalModel: reader cannot be null");
    }
    _current = _reader->root();
    if (!_current) {
        throw std::runtime_error("TerminalModel: lazy reader returned a null root");
    }
    ensureVisiblePage();
}

void TerminalModel::ensureVisiblePage() {
    _current->ensureChildrenLoaded(_selectedIndex + _pageSize);
}

std::shared_ptr<Node> TerminalModel::currentNode() const {
    return _current;
}

std::shared_ptr<Node> TerminalModel::selectedNode() const {
    return childAt(_current, _selectedIndex);
}

std::size_t TerminalModel::selectedIndex() const {
    return _selectedIndex;
}

const std::string& TerminalModel::statusMessage() const {
    return _statusMessage;
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

    return "  \033[36m" + iterator->second.text + "\033[0m";
}

void TerminalModel::moveSelection(const long long delta) {
    ensureVisiblePage();
    if (_current->loadedChildren().empty()) {
        _selectedIndex = 0;
        return;
    }

    if (delta < 0) {
        const std::size_t amount = static_cast<std::size_t>(-delta);
        _selectedIndex = amount > _selectedIndex ? 0 : _selectedIndex - amount;
        return;
    }

    const std::size_t amount = static_cast<std::size_t>(delta);
    const std::size_t target = _selectedIndex > std::numeric_limits<std::size_t>::max() - amount
        ? std::numeric_limits<std::size_t>::max()
        : _selectedIndex + amount;
    if (target < _current->loadedChildren().size()) {
        _selectedIndex = target;
        return;
    }

    if (_current->childrenLoadState() != ChildrenLoadState::Complete) {
        const std::size_t requested = target == std::numeric_limits<std::size_t>::max()
            ? target
            : target + 1;
        _current->ensureChildrenLoaded(requested);
        if (target < _current->loadedChildren().size()) {
            _selectedIndex = target;
            return;
        }
    }

    _selectedIndex = _current->loadedChildren().size() - 1;
}

void TerminalModel::enterSelectedChildren() {
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
    _statusMessage.clear();
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

    const Data& data = selected->data();
    if (data.size() > _payloadElementLimit) {
        _payloadDisplays[selected.get()] = PayloadDisplay{
            PayloadDisplay::State::TooBig,
            ""};
        _statusMessage = "Payload has " + std::to_string(data.size()) +
            " elements; display limit is " + std::to_string(_payloadElementLimit) + ".";
        return;
    }

    const std::string payloadText = data.hasString()
        ? data.extractString()
        : data.shortInfo();
    _payloadDisplays[selected.get()] = PayloadDisplay{
        PayloadDisplay::State::Displayed,
        payloadText};

    std::ostringstream stream;
    stream << selected->path() << " : " << selected->type() << "\n";
    stream << "payload (" << data.size() << " element(s), " << data.dtype() << "): ";
    stream << payloadText;
    _statusMessage = stream.str();
}

bool TerminalModel::handle(const Key key) {
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
        case Key::Left:
            leaveToParent();
            break;
        case Key::Right:
            enterSelectedChildren();
            break;
        case Key::Enter:
            enterSelectedPayload();
            break;
        case Key::Quit:
            return false;
        case Key::Unknown:
            break;
    }
    return true;
}

void TerminalModel::render(std::ostream& output) const {
    output << "\x1b[2J\x1b[H";
    output << "cgnsviz  " << _reader->filename() << "\n";
    output << "path: \033[1;4m" << _current->path() << "\033[0m\n";
    output << "type: " << _current->type()
           << "    children: " << loadStateName(_current->childrenLoadState())
           << "    loaded: " << _current->loadedChildren().size() << "\n\n";

    const auto& children = _current->loadedChildren();
    if (children.empty()) {
        output << "(no children)\n";
    } else {
        for (std::size_t index = 0; index < children.size(); ++index) {
            const auto& child = children[index];
            if (!child) {
                continue;
            }
            output << (index == _selectedIndex ? " >\033[7m" : "  ");
            output << child->name() << "  \033[90m" << child->type() << "\033[0m"
                   << payloadMarker(child) << "\n";
        }
        if (_current->childrenLoadState() != ChildrenLoadState::Complete) {
            output << "  ... more children available ...\n";
        }
    }

    output << "\n[Up/Down] select  [PgUp/PgDn] page  [Right] open  [Left] parent  [Enter] payload  [q] quit\n";
    if (!_statusMessage.empty()) {
        output << "\n" << _statusMessage << "\n";
    }
}

namespace {

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
            default: return Key::Unknown;
        }
    }
#else
    if (first == 27) {
        const int second = readByte();
        if (second != '[') {
            return Key::Unknown;
        }
        const int third = readByte();
        switch (third) {
            case 'A': return Key::Up;
            case 'B': return Key::Down;
            case 'C': return Key::Right;
            case 'D': return Key::Left;
            case '5':
                if (readByte() == '~') return Key::PageUp;
                return Key::Unknown;
            case '6':
                if (readByte() == '~') return Key::PageDown;
                return Key::Unknown;
            default: return Key::Unknown;
        }
    }
#endif

    if (first == '\r' || first == '\n') return Key::Enter;
    if (first == 'q' || first == 'Q') return Key::Quit;
    return Key::Unknown;
}

} // namespace

int runTerminal(TerminalModel& model, std::istream& input, std::ostream& output) {
    model.render(output);
    char value = 0;
    while (input.get(value)) {
        Key key = Key::Unknown;
        switch (value) {
            case 'k': key = Key::Up; break;
            case 'j': key = Key::Down; break;
            case 'h': key = Key::Left; break;
            case 'l': key = Key::Right; break;
            case 'q': key = Key::Quit; break;
            case '\n': case '\r': key = Key::Enter; break;
            default: break;
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
#endif
    AlternateScreen alternateScreen(std::cout);
    model.render(std::cout);
    std::cout.flush();
    while (true) {
        if (!model.handle(readKey())) {
            return 0;
        }
        model.render(std::cout);
        std::cout.flush();
    }
}

} // namespace cgnsviz::terminal
