#ifndef TREE_HPP
#define TREE_HPP

#include <memory>
#include <string>
#include <vector>

#include "cgns/base.hpp"
#include "cgns/zone.hpp"
#include "node/node.hpp"

#ifndef TREE_EXPORT
    #define TREE_EXPORT NODE_EXPORT
#endif

class TREE_EXPORT Tree : public Node {
public:
    using BaseList = std::vector<std::shared_ptr<Base>>;
    using ZoneList = std::vector<std::shared_ptr<Zone>>;
    using ZoneDataList = Base::ZoneDataList;
    using ZoneNamedDataList = Base::ZoneNamedDataList;

    Tree();

    BaseList bases() const;
    std::vector<std::string> baseNames() const;
    std::shared_ptr<Base> base(const std::string& name) const;
    void setUniqueBaseNames();
    void setUniqueZoneNames();
    void add(std::shared_ptr<Node> node);

    ZoneList zones() const;
    std::vector<std::string> zoneNames() const;

    bool isStructured() const;
    bool isUnstructured() const;
    bool isHybrid() const;

    size_t numberOfPoints() const;
    size_t numberOfCells() const;
    size_t numberOfZones() const;
    size_t numberOfBases() const;
    std::vector<std::string> getElementsTypes() const;

    ZoneDataList newFields(
        const std::vector<std::pair<std::string, double>>& inputFields,
        const std::string& container = "FlowSolution",
        const std::string& gridLocation = "auto",
        const std::string& dtype = "float64",
        bool ravel = false);

    ZoneDataList fields(
        const std::vector<std::string>& fieldNames,
        const std::string& container = "FlowSolution",
        const std::string& behaviorIfNotFound = "create",
        const std::string& dtype = "float64",
        bool ravel = false);

    ZoneNamedDataList allFields(
        bool includeCoordinates = true,
        bool appendContainerToFieldName = false,
        bool ravel = false) const;

    void assertFieldsSizeCoherency() const;
    void removeEmptyZones();
};

#endif
