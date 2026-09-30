#ifndef CGNSVIZ_TERMINAL_MODEL_HPP
#define CGNSVIZ_TERMINAL_MODEL_HPP

#include "node/node.hpp"

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace io::hdf5::cgns {
class LazyHdf5Reader;
}

namespace cgnsviz::terminal {

enum class Key {
    Up,
    Down,
    PageUp,
    PageDown,
    Home,
    End,
    Left,
    Right,
    Enter,
    ShiftEnter,
    SearchInward,
    SearchOutward,
    NextMatch,
    PreviousMatch,
    Quit,
    Unknown
};

/**
 * @brief State model for the non-search cgnsviz terminal view.
 *
 * The model contains no terminal I/O.  It provides deterministic navigation
 * and rendering over a lazy Node tree so that it can also be tested without
 * an interactive terminal.
 */
class TerminalModel {
public:
    TerminalModel(
        std::shared_ptr<io::hdf5::cgns::LazyHdf5Reader> reader,
        std::size_t pageSize = 5,
        std::size_t payloadElementLimit = 64,
        std::size_t maxPayloadChars = 80);

    /** @brief Apply one navigation key.  Returns false when the view should quit. */
    bool handle(Key key);

    /** @brief Render the current screen as plain text/ANSI-compatible text. */
    void render(std::ostream& output) const;

    /** @brief Set the available terminal height in rows. */
    void setViewportRows(std::size_t rows);

    /** @brief Currently displayed node (the parent of the selected row). */
    std::shared_ptr<Node> currentNode() const;

    /** @brief Currently selected child, or null when there are no children. */
    std::shared_ptr<Node> selectedNode() const;

    /** @brief Current selected row index. */
    std::size_t selectedIndex() const;

    /** @brief Last user-facing status or payload message. */
    const std::string& statusMessage() const;

    /** @brief Search from the selected node using one directional predicate block. */
    void search(const std::string& predicate, bool outward);

    /** @brief Select the next or previous result of the last search. */
    void nextSearchResult(bool previous = false);

private:
    struct PayloadDisplay {
        enum class State {
            Displayed,
            Summary,
            TooBig
        };

        State state;
        std::string text;
        std::string markerText;
    };

    void ensureVisiblePage();
    void ensureSelectionVisible();
    void moveSelection(long long delta);
    void selectFirstChild();
    void selectLastChild();
    void enterSelectedPayload();
    void enterSelectedPayloadView();
    void scrollPayload(long long delta);
    void leavePayloadView();
    void enterSelectedChildren();
    void leaveToParent();
    void selectSearchResult(const std::shared_ptr<Node>& node);
    void rememberPayload(const std::shared_ptr<Node>& node, const Data& data);
    std::string payloadMarker(const std::shared_ptr<Node>& node) const;
    std::size_t childViewportRows() const;
    std::size_t payloadViewportRows() const;
    void renderNodes(std::ostream& output) const;
    void renderPayload(std::ostream& output) const;

    std::shared_ptr<io::hdf5::cgns::LazyHdf5Reader> _reader;
    std::shared_ptr<Node> _current;
    std::size_t _selectedIndex;
    std::size_t _firstVisibleIndex;
    std::size_t _pageSize;
    std::size_t _payloadElementLimit;
    std::size_t _maxPayloadChars;
    std::size_t _viewportRows;
    std::string _statusMessage;
    std::unordered_map<const Node*, PayloadDisplay> _payloadDisplays;
    bool _payloadViewActive;
    std::shared_ptr<Node> _payloadNode;
    std::vector<std::string> _payloadLines;
    std::size_t _payloadScrollOffset;
    std::vector<std::shared_ptr<Node>> _searchResults;
    std::size_t _searchResultIndex;
    std::string _searchExpression;
};

/** @brief Run the platform-neutral key loop around a TerminalModel. */
int runTerminal(TerminalModel& model, std::istream& input, std::ostream& output);

/** @brief Run the real interactive terminal loop. */
int runInteractiveTerminal(TerminalModel& model);

} // namespace cgnsviz::terminal

#endif // CGNSVIZ_TERMINAL_MODEL_HPP
