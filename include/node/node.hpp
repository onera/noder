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

/**
 * @brief Recursive value container used by Node parameter APIs.
 *
 * This structure models Python-like parameter payloads:
 * - null-like placeholders
 * - leaf Data values
 * - ordered dictionaries
 * - ordered lists
 */
struct NODE_EXPORT ParameterValue {
    /** @brief Runtime variant discriminator. */
    enum class Kind {
        Null,
        Data,
        Dict,
        List
    };

    /** @brief Ordered dictionary entries. */
    using DictEntries = std::vector<std::pair<std::string, ParameterValue>>;
    /** @brief Ordered list entries. */
    using ListEntries = std::vector<ParameterValue>;

    /** @brief Active kind for this value. */
    Kind kind = Kind::Null;
    /** @brief Leaf payload when kind is Data. */
    std::shared_ptr<Data> data = nullptr;
    /** @brief Mapping payload when kind is Dict. */
    DictEntries dictEntries;
    /** @brief Sequence payload when kind is List. */
    ListEntries listEntries;

    /** @brief Build a null-like value. */
    static ParameterValue makeNull();
    /** @brief Build a leaf Data value. */
    static ParameterValue makeData(std::shared_ptr<Data> inputData);
    /** @brief Build an ordered dictionary value. */
    static ParameterValue makeDict(DictEntries entries);
    /** @brief Build an ordered list value. */
    static ParameterValue makeList(ListEntries entries);
};

/**
 * @brief Hierarchical node for CGNS-like tree structures.
 *
 * A Node stores:
 * - name/type metadata
 * - optional Data payload
 * - parent/children links
 * - optional external-link metadata
 */
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
    
    /**
     * @brief Register default payload factory used by Node constructor.
     * @param factory Callable returning a new default payload object.
     */
    static void setDefaultFactory(std::function<std::shared_ptr<Data>()> factory);

    /**
     * @brief Return this node and all descendants in depth-first order.
     * @return Ordered vector including this node first.
     */
    std::vector<std::shared_ptr<Node>> descendants(); // to be refactored into Navigation

    /**
     * @brief Construct a node.
     * @param name Node name.
     * @param type Node type string.
     */
    Node(const std::string& name = "", const std::string& type = "DataArray_t");

    ~Node();

    /** @brief Access navigation helper bound to this node. */
    Navigation& pick();
    
    /** @brief Shared pointer to this node (or null when unmanaged). */
    std::shared_ptr<Node> selfPtr();
    /** @brief Const shared pointer to this node (or null when unmanaged). */
    std::shared_ptr<const Node> selfPtr() const;

    // accessors and modifiers of class attributes
    /** @brief Node name accessor. */
    const std::string& name() const;
    /** @brief Rename this node. */
    void setName(const std::string& name);

    /** @brief Node type accessor. */
    std::string type() const;
    /** @brief Change node type string. */
    void setType(const std::string& type);
    /** @brief True when link target metadata is set. */
    bool hasLinkTarget() const;
    /** @brief Linked file for external/local link nodes. */
    const std::string& linkTargetFile() const;
    /** @brief Linked path for external/local link nodes. */
    const std::string& linkTargetPath() const;
    /**
     * @brief Mark node as link to another file/path target.
     * @param targetFile Target file ("." for same file).
     * @param targetPath Target path in that file.
     */
    void setLinkTarget(const std::string& targetFile, const std::string& targetPath);
    /** @brief Remove link target metadata. */
    void clearLinkTarget();

    /** @brief Parent accessor (weak to avoid cycles). */
    std::weak_ptr<Node> parent() const;

    /** @brief Read payload by reference. */
    const Data& data() const;
    /** @brief Read payload shared pointer. */
    std::shared_ptr<Data> dataPtr() const;

    /** @brief Replace payload with shared Data instance. */
    void setData(std::shared_ptr<Data> d);
    /** @brief Replace payload by cloning another Data object. */
    void setData(const Data& d);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    template <typename T, typename std::enable_if_t<utils::ContainsType_v<T, utils::ScalarTypes>, int> = 0>
    void setData(const T& d) {
        using CanonicalType = utils::CanonicalScalar_t<T>;
        this->setData(datafactory::makeDataFrom<CanonicalType>(static_cast<CanonicalType>(d)));
    }

    template <typename T, typename std::enable_if_t<std::is_same_v<utils::DecayCvRef_t<T>, std::string>, int> = 0>
    void setData(const T& d) {
        this->setData(datafactory::makeDataFrom<std::string>(d));
    }
#endif

    void setData(const char* d) {
        this->setData(datafactory::makeDataFrom(d));
    }

    /** @brief Human-readable summary of this node. */
    std::string getInfo() const;

    /** @brief True when payload is empty/none-like. */
    bool noData() const;

    /** @brief Child list accessor (in insertion order). */
    const std::vector<std::shared_ptr<Node>>& children() const;
    /** @brief True when node has at least one child. */
    bool hasChildren() const;
    /** @brief Siblings accessor with optional self inclusion. */
    std::vector<std::shared_ptr<Node>> siblings(bool includeMyself=true) const;
    /** @brief True when at least one sibling exists (excluding self). */
    bool hasSiblings() const;
    /** @brief Return child names in insertion order. */
    std::vector<std::string> getChildrenNames() const;
    
    /** @brief Return root ancestor for this node. */
    std::shared_ptr<const Node> root() const;

    /** @brief Depth level relative to root (root is 0). */
    size_t level() const;

    /** @brief Position among siblings (0-based). */
    size_t position() const;

    /** @brief Detach this node from current parent. */
    void detach();

    /**
     * @brief Attach this node below another parent node.
     * @param node New parent.
     * @param position Optional insertion index (-1 means append).
     * @param overrideSiblingByName Replace conflicting sibling when true.
     */
    void attachTo(
        std::shared_ptr<Node> node,
        const int16_t& position = -1,
        bool overrideSiblingByName = true);
    
    /** @brief Attach one child to this node. */
    void addChild(
        std::shared_ptr<Node> node,
        bool overrideSiblingByName = true,
        const int16_t& position = -1);
    /** @brief Attach multiple children to this node. */
    void addChildren(
        const std::vector<std::shared_ptr<Node>>& nodes,
        bool overrideSiblingByName = true);
    /** @brief Swap parent attachment with another node. */
    void swap(std::shared_ptr<Node> node);
    /** @brief Copy this subtree (deep copy duplicates payload buffers). */
    std::shared_ptr<Node> copy(bool deep=false) const;
    /** @brief Resolve a node from path (absolute or relative). */
    std::shared_ptr<Node> getAtPath(const std::string& path, bool pathIsRelative=false) const;
    /** @brief Collect all link descriptors under this subtree. */
    std::vector<std::tuple<std::string, std::string, std::string, std::string, int>> getLinks() const;
    /**
     * @brief Write/replace a parameter container node.
     * @param containerName Name of container child.
     * @param parameters Ordered key/value parameter entries.
     * @param containerType Type of container node.
     * @param parameterType Type used for leaf parameter nodes.
     * @return Created or updated container node.
     */
    std::shared_ptr<Node> setParameters(
        const std::string& containerName,
        const ParameterValue::DictEntries& parameters,
        const std::string& containerType = "UserDefinedData_t",
        const std::string& parameterType = "DataArray_t");
    /** @brief Read parameter container as recursive ParameterValue. */
    ParameterValue getParameters(const std::string& containerName) const;
    /** @brief Reload payload from disk using this node path as lookup key. */
    void reloadNodeData(const std::string& filename);
    /** @brief Save only this node payload/metadata to an existing file. */
    void saveThisNodeOnly(const std::string& filename, const std::string& backend = "hdf5");
    /** @brief Merge another subtree into this node (same-root strategy). */
    void merge(std::shared_ptr<Node> node);

    /** @brief Build absolute path from root to this node. */
    std::string path() const;

    #ifdef ENABLE_HDF5_IO
    /** @brief Write this subtree to a CGNS/HDF5 file. */
    void write(const std::string& filename);
    #endif // ENABLE_HDF5_IO

    // Print method
    /** @brief Stream helper for textual tree rendering. */
    void getPrintOutStream(std::ostream& os) const;
    /** @brief String representation used by Python bindings. */
    std::string __str__() const;
    /** @brief Render subtree as Unicode tree text. */
    std::string printTree(int max_depth=9999, std::string highlighted_path=std::string(""),
        int depth=0, bool last_pos=false, std::string markers=std::string("")) const;

    // Internal helpers for operator-expression semantics.
    /** @brief Internal expression-composition helper. */
    void setExpressionRepresentative(std::shared_ptr<Node> node);
    /** @brief Internal expression-composition accessor. */
    std::shared_ptr<Node> expressionRepresentative() const;

    friend std::ostream& operator<<(std::ostream& os, const Node& node);
    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node);

};

#endif
