#include "cgns/base.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

namespace {

size_t requireSizeTInInt32Range(size_t value, const char* context) {
    if (value > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
        throw std::runtime_error(std::string(context) + ": dimension exceeds int32_t range");
    }
    return value;
}

size_t dimensionFromBaseData(const Base& base, size_t index, const char* context) {
    auto data = base.dataPtr();
    if (!data || data->isNone() || data->dimensions() == 0 || data->size() <= index) {
        throw std::runtime_error(std::string(context) + ": base value must contain CellDimension and PhysicalDimension");
    }
    const int64_t value = data->itemAsInt64({index});
    if (value < 0) {
        throw std::runtime_error(std::string(context) + ": base dimensions cannot be negative");
    }
    return static_cast<size_t>(value);
}

bool hasCoordinate(const Zone& zone, const std::string& coordinateName) {
    auto gridCoordinates = const_cast<Zone&>(zone).pick().childByName("GridCoordinates");
    return gridCoordinates && gridCoordinates->pick().childByName(coordinateName);
}

size_t inferPhysicalDimension(const Zone& zone) {
    size_t dimension = 0;
    if (hasCoordinate(zone, "CoordinateX")) dimension = 1;
    if (hasCoordinate(zone, "CoordinateY")) dimension = 2;
    if (hasCoordinate(zone, "CoordinateZ")) dimension = 3;
    return dimension == 0 ? 3 : dimension;
}

} // namespace

Base::Base(const std::string& name) : Node(name.empty() ? "Base" : name, "CGNSBase_t") {
    this->setType("CGNSBase_t");
    this->setDimensions(3, 3);
}

void Base::setDimensions(size_t cellDimension, size_t physicalDimension) {
    requireSizeTInInt32Range(cellDimension, "Base::setDimensions");
    requireSizeTInInt32Range(physicalDimension, "Base::setDimensions");

    auto dimensions = this->data().full({2}, 0.0, "int32");
    dimensions->setItemFromInt64({0}, static_cast<int64_t>(cellDimension));
    dimensions->setItemFromInt64({1}, static_cast<int64_t>(physicalDimension));
    this->setData(dimensions);
}

size_t Base::dim() const {
    return dimensionFromBaseData(*this, 0, "Base::dim");
}

size_t Base::physicalDimension() const {
    return dimensionFromBaseData(*this, 1, "Base::physicalDimension");
}

void Base::setCellDimension(size_t cellDimension) {
    this->setDimensions(cellDimension, this->physicalDimension());
}

void Base::setPhysicalDimension(size_t physicalDimension) {
    this->setDimensions(this->dim(), physicalDimension);
}

Base::ZoneList Base::zones() const {
    ZoneList output;
    for (const auto& child : this->children()) {
        auto zone = std::dynamic_pointer_cast<Zone>(child);
        if (zone && zone->type() == "Zone_t") {
            output.push_back(zone);
        }
    }
    return output;
}

bool Base::isStructured() const {
    const auto zoneList = this->zones();
    return std::all_of(zoneList.begin(), zoneList.end(), [](const auto& zone) {
        return zone && zone->isStructured();
    });
}

bool Base::isUnstructured() const {
    const auto zoneList = this->zones();
    return std::all_of(zoneList.begin(), zoneList.end(), [](const auto& zone) {
        return zone && zone->isUnstructured();
    });
}

bool Base::isHybrid() const {
    return !this->isStructured() && !this->isUnstructured();
}

size_t Base::numberOfPoints() const {
    size_t total = 0;
    for (const auto& zone : this->zones()) {
        total += zone->numberOfPoints();
    }
    return total;
}

size_t Base::numberOfCells() const {
    size_t total = 0;
    for (const auto& zone : this->zones()) {
        total += zone->numberOfCells();
    }
    return total;
}

size_t Base::numberOfZones() const {
    return this->zones().size();
}

std::vector<std::string> Base::getElementsTypes() const {
    std::set<std::string> output;
    for (const auto& zone : this->zones()) {
        const auto zoneTypes = zone->getElementsTypes();
        output.insert(zoneTypes.begin(), zoneTypes.end());
    }
    return std::vector<std::string>(output.begin(), output.end());
}

Base::ZoneDataList Base::newFields(
    const std::vector<std::pair<std::string, double>>& inputFields,
    const std::string& container,
    const std::string& gridLocation,
    const std::string& dtype,
    bool ravel) {

    ZoneDataList output;
    for (const auto& zone : this->zones()) {
        output.emplace_back(
            zone->name(),
            zone->newFields(inputFields, container, gridLocation, dtype, ravel));
    }
    return output;
}

Base::ZoneDataList Base::fields(
    const std::vector<std::string>& fieldNames,
    const std::string& container,
    const std::string& behaviorIfNotFound,
    const std::string& dtype,
    bool ravel) {

    ZoneDataList output;
    for (const auto& zone : this->zones()) {
        output.emplace_back(
            zone->name(),
            zone->fields(fieldNames, container, behaviorIfNotFound, dtype, ravel));
    }
    return output;
}

Base::ZoneNamedDataList Base::allFields(
    bool includeCoordinates,
    bool appendContainerToFieldName,
    bool ravel) const {

    ZoneNamedDataList output;
    for (const auto& zone : this->zones()) {
        output.emplace_back(
            zone->name(),
            zone->allFields(includeCoordinates, appendContainerToFieldName, ravel));
    }
    return output;
}

void Base::assertFieldsSizeCoherency() const {
    for (const auto& zone : this->zones()) {
        zone->assertFieldsSizeCoherency();
    }
}

void Base::removeEmptyZones() {
    for (const auto& zone : this->zones()) {
        if (zone->isEmpty()) {
            zone->detach();
        }
    }
    this->updateDimensionsFromZones();
}

void Base::updateDimensionsFromZones() {
    const auto zoneList = this->zones();
    if (zoneList.empty()) {
        this->setDimensions(3, 3);
        return;
    }

    const size_t cellDimension = zoneList.front()->dim();
    const size_t physicalDimension = inferPhysicalDimension(*zoneList.front());
    for (const auto& zone : zoneList) {
        if (zone->dim() != cellDimension) {
            throw std::invalid_argument("newBase: all zones must have the same cell dimension");
        }
        if (inferPhysicalDimension(*zone) != physicalDimension) {
            throw std::invalid_argument("newBase: all zones must have the same physical dimension");
        }
    }

    this->setDimensions(cellDimension, physicalDimension);
}

std::shared_ptr<Base> newBase(
    const std::string& name,
    const std::vector<std::shared_ptr<Zone>>& zones) {

    auto base = std::make_shared<Base>(name);
    for (const auto& zone : zones) {
        if (!zone) {
            throw std::invalid_argument("newBase: zones cannot contain null entries");
        }
        base->addChild(zone, false);
    }
    base->updateDimensionsFromZones();
    return base;
}
