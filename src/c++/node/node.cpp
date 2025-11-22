#include "utils/compat.hpp"
#include "node/node.hpp"


// required for macOS, since cannot statically initialize
// dataFactory
// TODO find an alternative, since this breaks DIP
#ifdef __APPLE__
#include "array/array.hpp"
#endif


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
    _data(Node::dataFactory ? Node::dataFactory() : nullptr),
    _navigator(nullptr) {
#ifdef __APPLE__
    this->_data = std::make_shared<Array>();
#else
    this->_data = Node::dataFactory();
#endif
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

bool Node::noData() const {
    return _data->isNone();
}


const std::vector<std::shared_ptr<Node>>& Node::children() const {
    return _children;
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

void Node::attachTo(std::shared_ptr<Node> node) {
    if (!node) {
        throw std::invalid_argument("attachTo: Cannot attach to a null node");
    }

    bool uniqueSibling = true;
    auto& siblings = node->_children;

    for (const auto& sibling : siblings) {
        if (sibling->name() == this->name()) {
            uniqueSibling = false;
            break;
        }
    }

    if (uniqueSibling) {

        if (selfPtr()) {
            this->detach();
            this->_parent = node;
            node->_children.push_back(selfPtr());
        } else {
            throw std::runtime_error("attachTo: Stack-allocated nodes cannot be attached to heap-allocated nodes.");
        }

    } else {
        throw std::runtime_error("attachTo: Cannot attach '" + this->name() + "' to '" + node->name() +
                                 "' since a sibling with the same name already exists");
    }
}


void Node::addChild(std::shared_ptr<Node> node) {
    if (!node) {
        throw std::invalid_argument("addChild: Cannot add a null child");
    }
    node->attachTo(shared_from_this());
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

