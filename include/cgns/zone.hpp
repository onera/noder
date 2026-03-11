#ifndef ZONE_HPP
#define ZONE_HPP

# include <iostream>
# include <memory>
# include <functional>
# include <string>
# include <vector>
# include <array>
# include <algorithm>
# include <cstdint>
# include <variant>
# include <type_traits>
# include <utility>
# include <sstream>
# include <tuple>

# include "node/node.hpp"

// Zone is compiled in the same shared library as Node.
#ifndef ZONE_EXPORT
    #define ZONE_EXPORT NODE_EXPORT
#endif

/**
 * @brief CGNS Zone helper built on top of Node.
 *
 * The class mirrors a subset of treelab Zone capabilities and focuses on:
 * - zone dimensional metadata (points/cells)
 * - field container creation/access helpers
 * - coordinate shortcuts
 * - shape inference and boundary extraction for structured zones
 *
 * Not implemented by design: save(), useEquation().
 */
class ZONE_EXPORT Zone : public Node {
public:
    using NamedData = std::pair<std::string, std::shared_ptr<Data>>;
    using NamedDataList = std::vector<NamedData>;
    using DataList = std::vector<std::shared_ptr<Data>>;
    using ShapePair = std::pair<std::vector<size_t>, std::vector<size_t>>;
    using ZoneList = std::vector<std::shared_ptr<Zone>>;

    explicit Zone(const std::string& name = "Zone");

    bool isStructured() const;
    bool isUnstructured() const;
    std::vector<std::string> getElementsTypes() const;

    size_t dim() const;
    std::vector<size_t> shape() const;
    std::vector<size_t> shapeOfCoordinates() const;
    size_t numberOfPoints() const;
    size_t numberOfCells() const;

    DataList newFields(
        const std::vector<std::pair<std::string, double>>& inputFields,
        const std::string& container = "FlowSolution",
        const std::string& gridLocation = "auto",
        const std::string& dtype = "float64",
        bool ravel = false);

    void removeFields(
        const std::vector<std::string>& fieldNames,
        const std::string& container = "FlowSolution");

    DataList fields(
        const std::vector<std::string>& fieldNames,
        const std::string& container = "FlowSolution",
        const std::string& behaviorIfNotFound = "create",
        const std::string& dtype = "float64",
        bool ravel = false);

    std::shared_ptr<Data> field(
        const std::string& fieldName,
        const std::string& container = "FlowSolution",
        const std::string& behaviorIfNotFound = "create",
        const std::string& dtype = "float64",
        bool ravel = false);

    DataList xyz(bool ravel = false) const;
    DataList xy(bool ravel = false) const;
    DataList xz(bool ravel = false) const;
    DataList yz(bool ravel = false) const;
    std::shared_ptr<Data> x(bool ravel = false) const;
    std::shared_ptr<Data> y(bool ravel = false) const;
    std::shared_ptr<Data> z(bool ravel = false) const;

    NamedDataList allFields(
        bool includeCoordinates = true,
        bool appendContainerToFieldName = false,
        bool ravel = false) const;

    void assertFieldsSizeCoherency() const;
    std::string inferLocation(const std::string& container) const;
    bool hasFields() const;

    ShapePair getArrayShapes() const;
    void updateShape();
    bool isEmpty() const;

    ZoneList boundaries();
    std::shared_ptr<Zone> boundary(
        const std::string& index = "i",
        const std::string& bound = "min");
    std::shared_ptr<Zone> imin();
    std::shared_ptr<Zone> imax();
    std::shared_ptr<Zone> jmin();
    std::shared_ptr<Zone> jmax();
    std::shared_ptr<Zone> kmin();
    std::shared_ptr<Zone> kmax();

private:
    std::vector<std::array<int64_t, 3>> extractZoneShapeEntries(const char* context) const;
    std::vector<size_t> shapeAtLocation(const std::string& location, const char* context) const;

    std::shared_ptr<Node> ensureZoneTypeNode();
    std::shared_ptr<Node> zoneTypeNode() const;
    std::shared_ptr<Node> containerNode(const std::string& containerName) const;
    std::vector<std::shared_ptr<Node>> directChildrenByType(const std::string& nodeType) const;

    static std::string defaultGridLocationFor(const std::string& containerName);
};

#endif
