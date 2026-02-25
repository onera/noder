#include "utils/compat.hpp"
#include "data/data_factory.hpp"
#include "node/node.hpp"
#include "array/array.hpp"
#include <limits>

using namespace std::string_literals;

#include "io/io.hpp"


// // Unicode-compatible box characters
// #ifdef _WIN32
//     const std::string VERTICAL = "|   ";
//     const std::string BRANCH = "|__ ";
//     const std::string LAST_BRANCH = "|__ ";
// #else
const std::string VERTICAL = "│   ";
const std::string BRANCH = "├───";
const std::string LAST_BRANCH = "└───";
// #endif

namespace {

int16_t toAttachPosition(size_t position, const char* context) {
    if (position > static_cast<size_t>(std::numeric_limits<int16_t>::max())) {
        throw std::runtime_error(std::string(context) + ": sibling position exceeds int16_t range");
    }
    return static_cast<int16_t>(position);
}

std::vector<std::string> splitPathElements(const std::string& path) {
    std::vector<std::string> elements;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (!item.empty()) {
            elements.push_back(item);
        }
    }
    return elements;
}

std::shared_ptr<Node> findSiblingByNameExcluding(
    const std::shared_ptr<Node>& parent,
    const std::string& siblingName,
    const Node* ignoredNode) {

    for (const auto& sibling : parent->children()) {
        if (!sibling) {
            continue;
        }
        if (sibling.get() == ignoredNode) {
            continue;
        }
        if (sibling->name() == siblingName) {
            return sibling;
        }
    }
    return nullptr;
}

std::string nextUniqueSiblingName(
    const std::shared_ptr<Node>& parent,
    const std::string& baseName,
    const Node* ignoredNode) {

    int suffix = 0;
    while (true) {
        const std::string candidate = baseName + "." + std::to_string(suffix);
        if (!findSiblingByNameExcluding(parent, candidate, ignoredNode)) {
            return candidate;
        }
        suffix += 1;
    }
}

bool hasSiblingNameConflict(
    const std::shared_ptr<Node>& parent,
    const std::string& candidateName,
    const Node* ignoredA = nullptr,
    const Node* ignoredB = nullptr) {

    if (!parent) {
        return false;
    }

    for (const auto& sibling : parent->children()) {
        if (!sibling) {
            continue;
        }
        const Node* siblingRawPtr = sibling.get();
        if (siblingRawPtr == ignoredA || siblingRawPtr == ignoredB) {
            continue;
        }
        if (sibling->name() == candidateName) {
            return true;
        }
    }
    return false;
}

void mergeChildrenRecursively(
    const std::shared_ptr<Node>& mergedNode,
    const std::shared_ptr<Node>& incomingNode) {

    for (const auto& incomingChild : incomingNode->children()) {
        if (!incomingChild) {
            continue;
        }

        auto mergedChild = mergedNode->pick().childByName(incomingChild->name());
        if (mergedChild) {
            mergeChildrenRecursively(mergedChild, incomingChild);
        } else {
            mergedNode->addChild(incomingChild->copy());
        }
    }
}

} // namespace

std::function<std::shared_ptr<Data>()> Node::dataFactory;

void Node::setDefaultFactory(std::function<std::shared_ptr<Data>()> factory) {
    dataFactory = std::move(factory);
}

std::ostream& operator << ( std::ostream& os, const Node& node )  {
    node.getPrintOutStream(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node) {
    if (node) {
        return os << *node;
    } else {
        return os << "null";
    }
}

std::string Node::__str__() const {
    std::string txt = this->root()->printTree(9999,path());
    txt += this->getInfo();
    return txt; }

void Node::getPrintOutStream(std::ostream& os) const { 
    printUTF8Line(this->__str__(), os);
}


Node::Node(const std::string& name, const std::string& type) :
    _name(name),
    _children(),
    _type(type),
    _linkTargetFile(),
    _linkTargetPath(),
    _data(nullptr),
    _navigator(nullptr) {
    ensureFactoryInitialized();
    if (Node::dataFactory) {
        this->_data = Node::dataFactory();
    }

    if (!this->_data) {
        throw std::runtime_error("Node constructor: data factory is not initialized");
    }
}


// Destructor
Node::~Node() { 
    
    #ifndef NDEBUG
    std::cout << "entering destructor of " << this->name() << std::endl;
    #endif
    
    _children.clear();
    _parent.reset();

    # ifndef NDEBUG
    std::cout << "exiting destructor of " << this->name() << std::endl;
    # endif
 }


Navigation& Node::pick() {
    if (!_navigator) {
        _navigator = std::make_shared<Navigation>(*this);
    }
    return *_navigator;
}

std::shared_ptr<Node> Node::selfPtr() {
    try {
        return shared_from_this(); // if owned by shared_ptr
    } catch (const std::bad_weak_ptr&) {
        return nullptr; // if stack-allocated
    }
}

std::shared_ptr<const Node> Node::selfPtr() const {
    try {
        return shared_from_this();  
    } catch (const std::bad_weak_ptr&) {
        return nullptr;  
    }
}



// Accessors
const std::string& Node::name() const {
    return _name;
}

void Node::setName(const std::string& name) {
    this->_name = name;
}


const Data& Node::data() const {
    return *this->_data;
}

std::shared_ptr<Data> Node::dataPtr() const {
    return this->_data;
}

void Node::setData(std::shared_ptr<Data> d) {
    this->_data = std::move(d);
}

void Node::setData(const Data& d) {
    this->_data = d.clone();
}

std::string Node::type() const {
    return _type;
}

void Node::setType(const std::string& type) {
    this->_type = type;
}

bool Node::hasLinkTarget() const {
    return !_linkTargetPath.empty();
}

const std::string& Node::linkTargetFile() const {
    return _linkTargetFile;
}

const std::string& Node::linkTargetPath() const {
    return _linkTargetPath;
}

void Node::setLinkTarget(const std::string& targetFile, const std::string& targetPath) {
    if (targetPath.empty()) {
        throw std::invalid_argument("setLinkTarget: targetPath cannot be empty");
    }
    _linkTargetFile = targetFile;
    _linkTargetPath = targetPath;
}

void Node::clearLinkTarget() {
    _linkTargetFile.clear();
    _linkTargetPath.clear();
}

bool Node::noData() const {
    return _data->isNone();
}


const std::vector<std::shared_ptr<Node>>& Node::children() const {
    return _children;
}

bool Node::hasChildren() const {
    return !_children.empty();
}

std::vector<std::shared_ptr<Node>> Node::siblings(bool includeMyself) const {
    auto parentPtr = _parent.lock();
    if (!parentPtr) {
        if (includeMyself) {
            auto thisPtr = selfPtr();
            if (thisPtr) {
                return {std::const_pointer_cast<Node>(thisPtr)};
            }
        }
        return {};
    }

    std::vector<std::shared_ptr<Node>> siblingNodes;
    siblingNodes.reserve(parentPtr->children().size());

    for (const auto& sibling : parentPtr->children()) {
        if (!sibling) {
            continue;
        }
        if (!includeMyself && sibling.get() == this) {
            continue;
        }
        siblingNodes.push_back(sibling);
    }
    return siblingNodes;
}

bool Node::hasSiblings() const {
    return !siblings(false).empty();
}

std::vector<std::string> Node::getChildrenNames() const {
    std::vector<std::string> names;
    names.reserve(_children.size());
    for (const auto& child : _children) {
        if (child) {
            names.push_back(child->name());
        }
    }
    return names;
}


std::vector<std::shared_ptr<Node>> Node::descendants() {
    std::vector<std::shared_ptr<Node>> descendants;
    gatherAllDescendantsInList(shared_from_this(), descendants);
    return descendants;
}


void Node::gatherAllDescendantsInList(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& descendants) {
    descendants.push_back(node);
    for (const auto& child : node->children()) {
        gatherAllDescendantsInList(child, descendants);
    }
}


std::weak_ptr<Node> Node::parent() const {
    return _parent;
}


std::shared_ptr<const Node> Node::root() const {
    auto parent = this->_parent.lock();
    if (!parent) return shared_from_this();
    return parent->root();
}


size_t Node::level() const {
    size_t level = 0;
    std::weak_ptr<Node> weak_parent = _parent;

    while (auto parent = weak_parent.lock()) {
        level += 1;
        weak_parent = parent->_parent;
    }

    return level;
}

size_t Node::position() const {
    size_t pos = 0;

    std::shared_ptr<Node> parent = this->_parent.lock();
    if (!parent) return pos;

    auto& siblings_including_myself = parent->children();
    for (size_t i = 0; i < siblings_including_myself.size(); ++i) {
        if (siblings_including_myself[i].get() == this) {
            return i;
        }
    }

    throw std::runtime_error("position: Node is not found among its siblings");
}


void Node::detach() {
    
    std::shared_ptr<Node> parent = this->_parent.lock();

    if (parent) {
        auto& siblings = parent->_children;

        // remove_if is more efficient than for loop + if
        siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                                        [this](const std::shared_ptr<Node>& node) {
                                            return node.get() == this;
                                        }),
                        siblings.end());
    }
    this->_parent.reset();
}

void Node::attachTo(
    std::shared_ptr<Node> node,
    const int16_t& position,
    bool overrideSiblingByName) {

    if (!node) {
        throw std::invalid_argument("attachTo: Cannot attach to a null node");
    }
    auto thisPtr = selfPtr();
    if (!thisPtr) {
        throw std::runtime_error("attachTo: Stack-allocated nodes cannot be attached to heap-allocated nodes.");
    }

    auto siblingWithSameName = findSiblingByNameExcluding(node, this->name(), this);
    if (siblingWithSameName) {
        if (overrideSiblingByName) {
            siblingWithSameName->detach();
        } else {
            this->setName(nextUniqueSiblingName(node, this->name(), this));
        }
    }

    this->detach();
    this->_parent = node;

    auto& siblings = node->_children;
    const size_t nbOfSiblings = siblings.size();

    int32_t emplacementIndex = position;
    if (position < 0) {
        emplacementIndex = static_cast<int32_t>(nbOfSiblings) + position + 1;
    }

    if (emplacementIndex < 0 || static_cast<size_t>(emplacementIndex) > nbOfSiblings) {
        throw std::runtime_error(
            "attachTo: requested position " + std::to_string(position) +
            " produced emplacement index " + std::to_string(emplacementIndex) +
            " which is out of [0, " + std::to_string(nbOfSiblings) + "]");
    }

    node->_children.emplace(siblings.begin() + emplacementIndex, thisPtr);
}


void Node::addChild(
    std::shared_ptr<Node> node,
    bool overrideSiblingByName,
    const int16_t& position) {

    if (!node) {
        throw std::invalid_argument("addChild: Cannot add a null child");
    }
    node->attachTo(shared_from_this(), position, overrideSiblingByName);
}

void Node::addChildren(
    const std::vector<std::shared_ptr<Node>>& nodes,
    bool overrideSiblingByName) {

    for (const auto& node : nodes) {
        addChild(node, overrideSiblingByName);
    }
}

void Node::swap(std::shared_ptr<Node> node) {
    if (!node) {
        throw std::invalid_argument("swap: Cannot swap with a null node");
    }
    if (node.get() == this) {
        return;
    }

    auto thisParent = _parent.lock();
    auto otherParent = node->_parent.lock();

    if (thisParent && otherParent && thisParent.get() == otherParent.get()) {
        auto& siblings = thisParent->_children;
        auto thisIt = std::find_if(siblings.begin(), siblings.end(),
            [this](const std::shared_ptr<Node>& sibling) { return sibling.get() == this; });
        auto otherIt = std::find_if(siblings.begin(), siblings.end(),
            [node](const std::shared_ptr<Node>& sibling) { return sibling.get() == node.get(); });

        if (thisIt == siblings.end() || otherIt == siblings.end()) {
            throw std::runtime_error("swap: Could not locate both nodes in sibling list");
        }

        std::iter_swap(thisIt, otherIt);
        return;
    }

    if (hasSiblingNameConflict(thisParent, node->name(), this, node.get())) {
        throw std::runtime_error("swap: name conflict while attaching '" + node->name() +
                                 "' into '" + thisParent->path() + "'");
    }
    if (hasSiblingNameConflict(otherParent, this->name(), this, node.get())) {
        throw std::runtime_error("swap: name conflict while attaching '" + this->name() +
                                 "' into '" + otherParent->path() + "'");
    }

    const size_t thisPosition = position();
    const size_t otherPosition = node->position();

    this->detach();
    node->detach();

    if (otherParent) {
        this->attachTo(otherParent, toAttachPosition(otherPosition, "swap"));
    }
    if (thisParent) {
        node->attachTo(thisParent, toAttachPosition(thisPosition, "swap"));
    }
}

std::shared_ptr<Node> Node::copy(bool deep) const {
    auto copiedNode = std::make_shared<Node>(_name, _type);

    if (hasLinkTarget()) {
        copiedNode->setLinkTarget(_linkTargetFile, _linkTargetPath);
    }

    if (deep) {
        if (auto arrayData = std::dynamic_pointer_cast<Array>(_data)) {
            py::gil_scoped_acquire acquireGil;
            py::array copiedArray = arrayData->getPyArray().attr("copy")("K").cast<py::array>();
            copiedNode->setData(std::make_shared<Array>(copiedArray));
        } else {
            copiedNode->setData(_data->clone());
        }
    } else {
        copiedNode->setData(_data);
    }

    for (const auto& child : _children) {
        if (!child) {
            continue;
        }
        copiedNode->addChild(child->copy(deep));
    }

    return copiedNode;
}

std::shared_ptr<Node> Node::getAtPath(const std::string& path, bool pathIsRelative) const {
    if (path.empty()) {
        return nullptr;
    }

    std::shared_ptr<Node> startNode;
    if (pathIsRelative) {
        auto thisPtr = selfPtr();
        if (!thisPtr) {
            return nullptr;
        }
        startNode = std::const_pointer_cast<Node>(thisPtr);
    } else {
        auto rootPtr = root();
        if (!rootPtr) {
            return nullptr;
        }
        startNode = std::const_pointer_cast<Node>(rootPtr);
    }

    auto pathElements = splitPathElements(path);
    if (pathElements.empty()) {
        return nullptr;
    }

    size_t currentIndex = 0;
    if (pathElements.front() == startNode->name()) {
        currentIndex = 1;
    }

    auto currentNode = startNode;
    for (; currentIndex < pathElements.size(); ++currentIndex) {
        currentNode = currentNode->pick().childByName(pathElements[currentIndex]);
        if (!currentNode) {
            return nullptr;
        }
    }
    return currentNode;
}

void Node::merge(std::shared_ptr<Node> node) {
    if (!node) {
        throw std::invalid_argument("merge: Cannot merge a null node");
    }
    if (name() != node->name()) {
        throw std::invalid_argument("merge: Mismatching names '" + name() +
                                    "' and '" + node->name() + "'");
    }

    auto thisPtr = selfPtr();
    if (!thisPtr) {
        throw std::runtime_error("merge: Stack-allocated nodes are not supported");
    }

    mergeChildrenRecursively(thisPtr, node);
}


std::string Node::path() const {
    std::vector<std::string> ancestors;
    std::shared_ptr<Node> current = _parent.lock();

    while (current) {
        ancestors.push_back(current->name());
        current = current->_parent.lock();
    }

    std::reverse(ancestors.begin(), ancestors.end());

    std::string path;
    for (const auto& ancestor : ancestors) {
        if (!path.empty()) {
            path += "/";
        }
        path += ancestor;
    }

    if (!path.empty()) {
        path += "/";
    }
    path += this->name();

    return path;
}



std::string Node::printTree(int max_depth, std::string highlighted_path,
        int depth, bool last_pos, std::string markers) const {

    std::string this_markers;
    std::string fmt_name;
    std::string fmt_type;
    std::string fmt_value;
    
    bool useColor = isAnsiColorSupported();

    if (highlighted_path == path()) {
        fmt_name = useColor ? "\033[7m" + name() + "\033[0m" : name();
        fmt_type = useColor ? " \033[7;90m" + type() + "\033[0m" : " " + type();
        fmt_value = useColor ? " \033[7;36m" + this->_data->shortInfo() + "\033[0m"
                             : " " + this->_data->shortInfo();
    } else {
        fmt_name = name();
        fmt_type = useColor ? " \033[90m" + type() + "\033[0m" : " " + type();
        fmt_value = useColor ? " \033[36m" + this->_data->shortInfo() + "\033[0m"
                             : " " + this->_data->shortInfo();
    }

    if (depth > 0) {
        if (last_pos) {
            this_markers = markers+LAST_BRANCH;
            markers = markers+"    ";
        } else {
            this_markers = markers+BRANCH;
            markers = markers+VERTICAL;
        }
    } else {
        this_markers = "";
        markers = "";
    }


    std::string txt = this_markers + fmt_name + fmt_type + fmt_value + "\n";

    if (depth < max_depth) {
        auto myChildren  = children();
        size_t nbOfChildren = myChildren.size();
        for (size_t i = 0; i < nbOfChildren; i++) {
            auto child = myChildren[i];
            txt += child->printTree(max_depth, highlighted_path, depth + 1,
                                    i == nbOfChildren - 1, markers);
        }        
    }

    return txt;
}


std::string Node::getInfo() const {
    
    std::string txt;
    txt += "path: " + path() + "\n";
    txt += "type: " + type() + "\n";

    
    if (this->noData()) {
        txt += "no data\n";
    } else {
        txt += this->data().info();    
    }
    return txt;
    
}


#ifdef ENABLE_HDF5_IO
void Node::write(const std::string& filename) {
    io::write_node(filename, shared_from_this());
}
#endif // ENABLE_HDF5_IO
