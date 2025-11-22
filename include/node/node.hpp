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

# include "data/data.hpp"
# include "node/navigation.hpp"
# include "utils/compat.hpp"

class Navigation;

class NODE_EXPORT Node : public std::enable_shared_from_this<Node> {

private:

    std::string _name;
    std::vector<std::shared_ptr<Node>> _children;
    std::string _type;
    std::weak_ptr<Node> _parent;
    std::shared_ptr<Data> _data;
    static std::function<std::shared_ptr<Data>()> dataFactory;

    mutable std::shared_ptr<Navigation> _navigator;


    void gatherAllDescendantsInList( std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& descendants);

public:
    
    static void setDefaultFactory(std::function<std::shared_ptr<Data>()> factory);

    std::vector<std::shared_ptr<Node>> getAllDescendants(); // to be refactored into Navigation

    Node(const std::string& name = "", const std::string& type = "DataArray_t");

    ~Node();

    Navigation& pick();
    
    std::shared_ptr<Node> selfPtr();
    std::shared_ptr<const Node> selfPtr() const;

    // accessors and modifiers of class attributes
    const std::string& name() const;
    void setName(const std::string& name);

    std::string type() const;
    void setType(const std::string& type);

    std::weak_ptr<Node> parent() const;

    const Data& data() const;
    std::shared_ptr<Data> dataPtr() const;

    void setData(std::shared_ptr<Data> d);
    void setData(const Data& d);

    std::string getInfo() const;

    bool noData() const;

    const std::vector<std::shared_ptr<Node>>& children() const;
    
    std::shared_ptr<const Node> root() const;

    size_t level() const;

    size_t positionAmongSiblings() const;

    void detach();

    void attachTo(std::shared_ptr<Node> node);
    
    void addChild(std::shared_ptr<Node> node);

    std::string path() const;

    #ifdef ENABLE_HDF5_IO
    void write(const std::string& filename);
    #endif // ENABLE_HDF5_IO

    // Print method
    void getPrintOutStream(std::ostream& os) const;
    std::string __str__() const;
    std::string printTree(int max_depth=9999, std::string highlighted_path=std::string(""),
        int depth=0, bool last_pos=false, std::string markers=std::string("")) const;

    friend std::ostream& operator<<(std::ostream& os, const Node& node);
    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node);

};


#endif
