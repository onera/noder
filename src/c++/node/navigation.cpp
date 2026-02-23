#include "node/navigation.hpp"
#include "node/node.hpp"

namespace {

std::string globToRegexPattern(const std::string& globPattern) {
    std::string regexPattern;
    regexPattern.reserve(globPattern.size() * 2 + 2);
    regexPattern += "^";

    for (char c : globPattern) {
        switch (c) {
            case '*':
                regexPattern += ".*";
                break;
            case '?':
                regexPattern += ".";
                break;
            case '.':
            case '^':
            case '$':
            case '+':
            case '(':
            case ')':
            case '{':
            case '}':
            case '[':
            case ']':
            case '|':
            case '\\':
                regexPattern += "\\";
                regexPattern += c;
                break;
            default:
                regexPattern += c;
                break;
        }
    }

    regexPattern += "$";
    return regexPattern;
}

} // namespace

Navigation::Navigation(Node& inputNode) : _node(inputNode) {}

std::shared_ptr<Node> Navigation::childByName(const std::string& name) {

    for (auto child: _node.children()) {
        if (child && name == child->name()) return child;
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byName(const std::string& name, const size_t& depth) {

    if ( depth > 0 ) {
        if ( _node.name() == name ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byName(name, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByName(const std::string& name, const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        if ( _node.name() == name ) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByName(name, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byNameRegex(const std::string& namePattern, const size_t& depth) {

    if ( depth > 0 ) {
        std::regex regexPattern(namePattern);
        if (std::regex_search(_node.name(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byNameRegex(namePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByNameRegex(
    const std::string& namePattern,
    const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        std::regex regexPattern(namePattern);
        if (std::regex_search(_node.name(), regexPattern)) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByNameRegex(namePattern, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byNameGlob(const std::string& namePattern, const size_t& depth) {
    if ( depth > 0 ) {
        const std::regex regexPattern(globToRegexPattern(namePattern));
        if (std::regex_match(_node.name(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byNameGlob(namePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByNameGlob(
    const std::string& namePattern,
    const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        const std::regex regexPattern(globToRegexPattern(namePattern));
        if (std::regex_match(_node.name(), regexPattern)) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByNameGlob(namePattern, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}


std::shared_ptr<Node> Navigation::childByType(const std::string& type) {

    for (auto child: _node.children()) {
        if (child && type == child->type()) return child;
    }
    return nullptr;
}


std::shared_ptr<Node> Navigation::byType(const std::string& type, const size_t& depth) {

    if ( depth > 0 ) {
        if ( _node.type() == type ) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byType(type, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByType(const std::string& type, const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        if ( _node.type() == type ) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByType(type, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}


std::shared_ptr<Node> Navigation::byTypeRegex(const std::string& typePattern,
                                                const size_t& depth) {
    if ( depth > 0 ) {
        std::regex regexPattern(typePattern);
        if (std::regex_search(_node.type(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byTypeRegex(typePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByTypeRegex(
    const std::string& typePattern,
    const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        std::regex regexPattern(typePattern);
        if (std::regex_search(_node.type(), regexPattern)) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByTypeRegex(typePattern, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byTypeGlob(const std::string& typePattern,
                                             const size_t& depth) {
    if ( depth > 0 ) {
        const std::regex regexPattern(globToRegexPattern(typePattern));
        if (std::regex_match(_node.type(), regexPattern)) return _node.shared_from_this();

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byTypeGlob(typePattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByTypeGlob(
    const std::string& typePattern,
    const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        const std::regex regexPattern(globToRegexPattern(typePattern));
        if (std::regex_match(_node.type(), regexPattern)) {
            matches.push_back(_node.shared_from_this());
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByTypeGlob(typePattern, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::childByData(const std::string& data) {

    for (auto child: _node.children()) {
        if (child) {

            bool isDataString = child->data().hasString();
            
            if (isDataString) {
                std::string dataAsString = child->data().extractString();

                if (data == dataAsString) return child;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::childByData(const char* data) {
    std::string data_str = std::string(data);
    return this->childByData(data_str);
}

template <typename T>
std::shared_ptr<Node> Navigation::childByData(const T& data) {

    for (auto child: _node.children()) {
        if (child) {

            bool isScalar = child->data().isScalar();
            
            if (isScalar) {
                if (child->data() == data) return child;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byData(const std::string& data, const size_t& depth) {

    if ( depth > 0 ) {

        bool thisHasDataString = _node.data().hasString();

        if (thisHasDataString) {
            if (_node.data().extractString() == data ) {
                return _node.shared_from_this();
            }
        }

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byData(data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByData(const std::string& data, const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {
        bool thisHasDataString = _node.data().hasString();

        if (thisHasDataString) {
            if (_node.data().extractString() == data ) {
                matches.push_back(_node.shared_from_this());
            }
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByData(data, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byDataGlob(const std::string& dataPattern, const size_t& depth) {
    if ( depth > 0 ) {

        const std::regex regexPattern(globToRegexPattern(dataPattern));

        bool thisHasDataString = _node.data().hasString();
        if (thisHasDataString) {
            const std::string dataAsString = _node.data().extractString();
            if (std::regex_match(dataAsString, regexPattern)) {
                return _node.shared_from_this();
            }
        }

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byDataGlob(dataPattern, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByDataGlob(
    const std::string& dataPattern,
    const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {

        const std::regex regexPattern(globToRegexPattern(dataPattern));

        bool thisHasDataString = _node.data().hasString();
        if (thisHasDataString) {
            const std::string dataAsString = _node.data().extractString();
            if (std::regex_match(dataAsString, regexPattern)) {
                matches.push_back(_node.shared_from_this());
            }
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByDataGlob(dataPattern, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byData(const char* data, const size_t& depth) {
    std::string data_str = std::string(data);
    return this->byData(data_str, depth);
}

std::vector<std::shared_ptr<Node>> Navigation::allByData(const char* data, const size_t& depth) {
    std::string data_str = std::string(data);
    return this->allByData(data_str, depth);
}

template <typename T>
std::shared_ptr<Node> Navigation::byData(const T& data, const size_t& depth) {

    if ( depth > 0 ) {

        if (_node.data().isScalar()) {
            if (_node.data() == data ) {
                return _node.shared_from_this();
            }
        }

        for (auto child: _node.children()) {
            auto foundNode = child->pick().byData(data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

template <typename T>
std::vector<std::shared_ptr<Node>> Navigation::allByData(const T& data, const size_t& depth) {
    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {

        if (_node.data().isScalar()) {
            if (_node.data() == data ) {
                matches.push_back(_node.shared_from_this());
            }
        }

        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByData(data, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

std::shared_ptr<Node> Navigation::byAnd(
    const std::string& name,
    const std::string& type,
    const std::string& data,
    const size_t& depth) {

    if ( depth > 0 ) {

        bool matchName(false);
        bool matchType(false);
        bool matchData(false);

        if ( name.empty() || _node.name() == name ) {
            matchName = true;
            
            if ( type.empty() || _node.type() == type ) {
                matchType = true;

                if ( data.empty() ) {
                    matchData = true;
                } else if (_node.data().hasString()) {
                    std::string data_str = _node.data().extractString();
                    matchData = data_str == data;
                }
            }
        }

        if ( matchName && matchType && matchData ) return _node.shared_from_this();


        for (auto child: _node.children()) {
            auto foundNode = child->pick().byAnd(name, type, data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::shared_ptr<Node> Navigation::byAnd(
    const char* name,
    const char* type,
    const char* data,
    const size_t& depth) {
        return Navigation::byAnd(std::string(name),
                                 std::string(type),
                                 std::string(data), depth);
}


template <typename T>
std::shared_ptr<Node> Navigation::byAnd(
    const std::string& name,
    const std::string& type,
    const T& data,
    const size_t& depth) {

    if ( depth > 0 ) {

        bool matchName(false);
        bool matchType(false);
        bool matchData(false);

        if ( name.empty() || _node.name() == name ) {
            matchName = true;
            
            if ( type.empty() || _node.type() == type ) {
                matchType = true;

                if ( _node.data().isScalar() && _node.data() == data ) {
                    matchData = true;
                }
            }
        }

        if ( matchName && matchType && matchData ) return _node.shared_from_this();


        for (auto child: _node.children()) {
            auto foundNode = child->pick().byAnd(name, type, data, depth-1);
            if (foundNode) return foundNode;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Navigation::allByAnd(
    const std::string& name,
    const std::string& type,
    const std::string& data,
    const size_t& depth) {
    return this->allByAnd<std::string>(name, type, data, depth);
}

std::vector<std::shared_ptr<Node>> Navigation::allByAnd(
    const char* name,
    const char* type,
    const char* data,
    const size_t& depth) {
    return this->allByAnd(std::string(name),
                          std::string(type),
                          std::string(data),
                          depth);
}

template <typename T>
std::vector<std::shared_ptr<Node>> Navigation::allByAnd(
    const std::string& name,
    const std::string& type,
    const T& data,
    const size_t& depth) {

    std::vector<std::shared_ptr<Node>> matches;

    if ( depth > 0 ) {

        bool matchName(false);
        bool matchType(false);
        bool matchData(false);

        if ( name.empty() || _node.name() == name ) {
            matchName = true;
            
            if ( type.empty() || _node.type() == type ) {
                matchType = true;

                if constexpr (std::is_same_v<T, std::string>) {
                    if ( data.empty() ) {
                        matchData = true;
                    } else if (_node.data().hasString()) {
                        std::string data_str = _node.data().extractString();
                        matchData = data_str == data;
                    }
                }  else if (_node.data().isScalar()) {
                    matchData = _node.data() == data;
                }
            }
        }

        if ( matchName && matchType && matchData ) {
            matches.push_back(_node.shared_from_this());
        };


        for (auto child: _node.children()) {
            auto childMatches = child->pick().allByAnd(name, type, data, depth-1);
            matches.insert(matches.end(), childMatches.begin(), childMatches.end());
        }
    }
    return matches;
}

/*
    template instantiations
*/

template <typename... T>
struct InstantiatorMethodScalar {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&Navigation::template childByData<U>), ...);
        (utils::forceSymbol(&Navigation::template byData<U>), ...);
        (utils::forceSymbol(&Navigation::template allByData<U>), ...);
        (utils::forceSymbol(&Navigation::template byAnd<U>), ...);
        (utils::forceSymbol(&Navigation::template allByAnd<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorMethodScalar, utils::ScalarTypes>();
template std::shared_ptr<Node> Navigation::childByData<bool>(const bool&);
template std::shared_ptr<Node> Navigation::childByData<double>(const double&);
template std::shared_ptr<Node> Navigation::childByData<int>(const int&);
template std::shared_ptr<Node> Navigation::byData<bool>(const bool&, const size_t&);
template std::shared_ptr<Node> Navigation::byData<double>(const double&, const size_t&);
template std::shared_ptr<Node> Navigation::byData<int>(const int&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByData<bool>(const bool&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByData<double>(const double&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByData<int>(const int&, const size_t&);
template std::shared_ptr<Node> Navigation::byAnd<bool>(const std::string&, const std::string&, const bool&, const size_t&);
template std::shared_ptr<Node> Navigation::byAnd<double>(const std::string&, const std::string&, const double&, const size_t&);
template std::shared_ptr<Node> Navigation::byAnd<int>(const std::string&, const std::string&, const int&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByAnd<bool>(const std::string&, const std::string&, const bool&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByAnd<double>(const std::string&, const std::string&, const double&, const size_t&);
template std::vector<std::shared_ptr<Node>> Navigation::allByAnd<int>(const std::string&, const std::string&, const int&, const size_t&);
