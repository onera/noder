#ifndef BASE_HPP
#define BASE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cgns/zone.hpp"
#include "node/node.hpp"

#ifndef BASE_EXPORT
    #define BASE_EXPORT NODE_EXPORT
#endif

class BASE_EXPORT Base : public Node {
public:
    using ZoneList = std::vector<std::shared_ptr<Zone>>;
    using ZoneDataList = std::vector<std::pair<std::string, Zone::DataList>>;
    using ZoneNamedDataList = std::vector<std::pair<std::string, Zone::NamedDataList>>;

    explicit Base(const std::string& name = "Base");

    size_t dim() const;
    size_t physicalDimension() const;
    void setCellDimension(size_t cellDimension);
    void setPhysicalDimension(size_t physicalDimension);

    ZoneList zones() const;
    bool isStructured() const;
    bool isUnstructured() const;
    bool isHybrid() const;

    size_t numberOfPoints() const;
    size_t numberOfCells() const;
    size_t numberOfZones() const;
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
    void updateDimensionsFromZones();

private:
    void setDimensions(size_t cellDimension, size_t physicalDimension);
};

std::shared_ptr<Base> newBase(
    const std::string& name,
    const std::vector<std::shared_ptr<Zone>>& zones);

#endif
