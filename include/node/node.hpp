#ifndef NODE_HPP
#define NODE_HPP

// visibility parameters for exporting symbols, since Node is used as a library
#ifndef NODE_EXPORT
    #if defined(_WIN32) || defined(__CYGWIN__)
        #ifdef BUILDING_NODE_LIBRARY
            #define NODE_EXPORT __declspec(dllexport)
        #else
            #define NODE_EXPORT __declspec(dllimport)
        #endif
    #elif __GNUC__ >= 4
        #define NODE_EXPORT __attribute__((visibility("default")))
    #else
        #define NODE_EXPORT
    #endif
#endif


# include <iostream>
# include <memory>
# include <functional>
# include <string>
# include <vector>
# include <algorithm>
# include <cstdint>
# include <variant>
# include <type_traits>
# include <utility>
# include <sstream>
# include <tuple>

# include "data/data.hpp"
# include "node/navigation.hpp"
# include "node/node_group.hpp"
# include "utils/data_types.hpp"
# include "utils/compat.hpp"

class Navigation;

# ifndef DATA_FACTORY_HPP
namespace datafactory {
    template <typename T>
    std::shared_ptr<Data> makeDataFrom(const T& value);

    std::shared_ptr<Data> makeDataFrom(const char* value);
}
# endif

struct NODE_EXPORT ParameterValue {
    /// Supported parameter container kinds used to map between Python objects and Node trees.
    enum class Kind {
        /// No value; corresponds to Python `None` or placeholder-like values.
        Null,
        /// Leaf value represented by a `Data` instance.
        Data,
        /// Mapping of named parameter entries.
        Dict,
        /// Ordered list of parameter entries.
        List
    };

    /// Ordered key/value pairs for dictionary-like parameters.
    using DictEntries = std::vector<std::pair<std::string, ParameterValue>>;
    /// Ordered values for list-like parameters.
    using ListEntries = std::vector<ParameterValue>;

    /// Active kind stored in this container.
    Kind kind = Kind::Null;
    /// Leaf data payload used when `kind == Kind::Data`.
    std::shared_ptr<Data> data = nullptr;
    /// Dictionary payload used when `kind == Kind::Dict`.
    DictEntries dictEntries;
    /// List payload used when `kind == Kind::List`.
    ListEntries listEntries;

    /// Build a null parameter value.
    static ParameterValue makeNull();
    /// Build a data parameter value from an existing `Data` pointer.
    static ParameterValue makeData(std::shared_ptr<Data> inputData);
    /// Build a dictionary parameter value preserving insertion order.
    static ParameterValue makeDict(DictEntries entries);
    /// Build a list parameter value preserving insertion order.
    static ParameterValue makeList(ListEntries entries);
};

/// Hierarchical CGNS-like node storing typed metadata, optional payload and children.
class NODE_EXPORT Node : public std::enable_shared_from_this<Node> {

private:

    std::string _name;
    std::vector<std::shared_ptr<Node>> _children;
    std::string _type;
    std::weak_ptr<Node> _parent;
    std::weak_ptr<Node> _expressionRepresentative;
    std::shared_ptr<Data> _data;
    std::string _linkTargetFile;
    std::string _linkTargetPath;
    static std::function<std::shared_ptr<Data>()> dataFactory;

    mutable std::shared_ptr<Navigation> _navigator;


    void gatherAllDescendantsInList( std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& descendants);

public:
    
    /// Set the global default factory used to initialize node payloads.
    static void setDefaultFactory(std::function<std::shared_ptr<Data>()> factory);

    /// Return this node and all descendants in depth-first order.
    std::vector<std::shared_ptr<Node>> descendants(); // to be refactored into Navigation

    /// Construct a node with a name and type.
    Node(const std::string& name = "", const std::string& type = "DataArray_t");

    ~Node();

    /// Access the navigation helper bound to this node.
    Navigation& pick();
    
    /// Return `shared_ptr` to self when managed by shared ownership; otherwise null.
    std::shared_ptr<Node> selfPtr();
    /// Const-qualified variant of `selfPtr`.
    std::shared_ptr<const Node> selfPtr() const;

    // accessors and modifiers of class attributes
    /// Node name.
    const std::string& name() const;
    /// Set node name.
    void setName(const std::string& name);

    /// Node type string.
    std::string type() const;
    /// Set node type string.
    void setType(const std::string& type);
    /// Whether this node holds link metadata.
    bool hasLinkTarget() const;
    /// Link target file component.
    const std::string& linkTargetFile() const;
    /// Link target path component.
    const std::string& linkTargetPath() const;
    /// Set link target metadata.
    void setLinkTarget(const std::string& targetFile, const std::string& targetPath);
    /// Clear link target metadata.
    void clearLinkTarget();

    /// Parent node (weak ownership).
    std::weak_ptr<Node> parent() const;

    /// Access payload as `Data` reference.
    const Data& data() const;
    /// Access payload pointer.
    std::shared_ptr<Data> dataPtr() const;

    /// Set payload from `Data` pointer.
    void setData(std::shared_ptr<Data> d);
    /// Set payload from cloned `Data` value.
    void setData(const Data& d);

    /// Set payload from scalar value.
    template <typename T, typename std::enable_if_t<utils::ContainsType_v<T, utils::ScalarTypes>, int> = 0>
    void setData(const T& d) {
        using CanonicalType = utils::CanonicalScalar_t<T>;
        this->setData(datafactory::makeDataFrom<CanonicalType>(static_cast<CanonicalType>(d)));
    }

    /// Set payload from string value.
    template <typename T, typename std::enable_if_t<std::is_same_v<utils::DecayCvRef_t<T>, std::string>, int> = 0>
    void setData(const T& d) {
        this->setData(datafactory::makeDataFrom<std::string>(d));
    }

    /// Set payload from C string.
    void setData(const char* d) {
        this->setData(datafactory::makeDataFrom(d));
    }

    /// Human-readable debug description of this node.
    std::string getInfo() const;

    /// Whether node has no payload.
    bool noData() const;

    /// Child list.
    const std::vector<std::shared_ptr<Node>>& children() const;
    /// Whether node has children.
    bool hasChildren() const;
    /// Siblings, optionally including self.
    std::vector<std::shared_ptr<Node>> siblings(bool includeMyself=true) const;
    /// Whether node has siblings (excluding self).
    bool hasSiblings() const;
    /// Child names preserving child order.
    std::vector<std::string> getChildrenNames() const;
    
    /// Root ancestor.
    std::shared_ptr<const Node> root() const;

    /// Depth from root.
    size_t level() const;

    /// Position among siblings.
    size_t position() const;

    /// Detach node from parent.
    void detach();

    /// Attach node to a new parent.
    void attachTo(
        std::shared_ptr<Node> node,
        const int16_t& position = -1,
        bool overrideSiblingByName = true);
    
    /// Add one child.
    void addChild(
        std::shared_ptr<Node> node,
        bool overrideSiblingByName = true,
        const int16_t& position = -1);
    /// Add multiple children.
    void addChildren(
        const std::vector<std::shared_ptr<Node>>& nodes,
        bool overrideSiblingByName = true);
    /// Swap positions with another node.
    void swap(std::shared_ptr<Node> node);
    /// Copy subtree; optionally deep-copy payloads.
    std::shared_ptr<Node> copy(bool deep=false) const;
    /// Resolve node by path.
    std::shared_ptr<Node> getAtPath(const std::string& path, bool pathIsRelative=false) const;
    /// Collect link metadata in CGNS-compatible tuple layout.
    std::vector<std::tuple<std::string, std::string, std::string, std::string, int>> getLinks() const;
    /// Write a parameter container from ordered key/value entries.
    std::shared_ptr<Node> setParameters(
        const std::string& containerName,
        const ParameterValue::DictEntries& parameters,
        const std::string& containerType = "UserDefinedData_t",
        const std::string& parameterType = "DataArray_t");
    /// Read a parameter container as recursive `ParameterValue`.
    ParameterValue getParameters(const std::string& containerName) const;
    /// Reload this node payload from file at the same path.
    void reloadNodeData(const std::string& filename);
    /// Persist only this node payload/metadata into an existing file.
    void saveThisNodeOnly(const std::string& filename, const std::string& backend = "hdf5");
    /// Merge descendants from another node with same name.
    void merge(std::shared_ptr<Node> node);

    /// Full path from root.
    std::string path() const;

    #ifdef ENABLE_HDF5_IO
    /// Write subtree to file.
    void write(const std::string& filename);
    #endif // ENABLE_HDF5_IO

    // Print method
    /// Stream pretty-print helper.
    void getPrintOutStream(std::ostream& os) const;
    /// Python-like string representation.
    std::string __str__() const;
    /// Render subtree as text tree.
    std::string printTree(int max_depth=9999, std::string highlighted_path=std::string(""),
        int depth=0, bool last_pos=false, std::string markers=std::string("")) const;

    // Internal helpers for operator-expression semantics.
    /// Set expression representative used by operator composition.
    void setExpressionRepresentative(std::shared_ptr<Node> node);
    /// Get current expression representative.
    std::shared_ptr<Node> expressionRepresentative() const;

    friend std::ostream& operator<<(std::ostream& os, const Node& node);
    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node);

};

#endif
