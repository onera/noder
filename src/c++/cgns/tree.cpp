#include "cgns/tree.hpp"

#include <algorithm>
#include <set>

Tree::Tree() : Node("CGNSTree", "CGNSTree_t") {
    this->setType("CGNSTree_t");
}

std::shared_ptr<Node> Tree::makeCopyShell() const {
    auto result = std::make_shared<Tree>();
    result->setName(name());
    result->setType(type());
    return result;
}

Tree::BaseList Tree::bases() const {
    BaseList output;
    for (const auto& child : this->children()) {
        auto base = std::dynamic_pointer_cast<Base>(child);
        if (base && base->type() == "CGNSBase_t") {
            output.push_back(base);
        }
    }
    return output;
}

std::vector<std::string> Tree::baseNames() const {
    std::vector<std::string> output;
    for (const auto& base : this->bases()) {
        output.push_back(base->name());
    }
    return output;
}

std::shared_ptr<Base> Tree::base(const std::string& name) const {
    for (const auto& base : this->bases()) {
        if (base->name() == name) {
            return base;
        }
    }
    return nullptr;
}

void Tree::setUniqueBaseNames() {
    std::set<std::string> usedNames;
    for (const auto& base : this->bases()) {
        const std::string baseName = base->name();
        if (usedNames.insert(baseName).second) {
            continue;
        }

        size_t index = 0;
        std::string candidate;
        do {
            candidate = baseName + "." + std::to_string(index++);
        } while (usedNames.find(candidate) != usedNames.end());

        base->setName(candidate);
        usedNames.insert(candidate);
    }
}

void Tree::setUniqueZoneNames() {
    std::set<std::string> usedNames;
    for (const auto& zone : this->zones()) {
        const std::string zoneName = zone->name();
        if (usedNames.insert(zoneName).second) {
            continue;
        }

        size_t index = 0;
        std::string candidate;
        do {
            candidate = zoneName + "." + std::to_string(index++);
        } while (usedNames.find(candidate) != usedNames.end());

        zone->setName(candidate);
        usedNames.insert(candidate);
    }
}

void Tree::add(std::shared_ptr<Node> node) {
    if (!node) {
        return;
    }

    if (auto tree = std::dynamic_pointer_cast<Tree>(node)) {
        const auto children = tree->children();
        this->addChildren(children, false);
    } else if (std::dynamic_pointer_cast<Base>(node)) {
        this->addChild(node, false);
    } else if (auto zone = std::dynamic_pointer_cast<Zone>(node)) {
        auto baseList = this->bases();
        std::shared_ptr<Base> targetBase;
        if (baseList.empty()) {
            targetBase = newBase("Base", {});
            this->addChild(targetBase, false);
        } else {
            targetBase = baseList.back();
        }
        targetBase->addChild(zone, false);
        targetBase->updateDimensionsFromZones();
    } else {
        this->addChild(node, false);
    }

    this->setUniqueBaseNames();
}

Tree::ZoneList Tree::zones() const {
    ZoneList output;
    for (const auto& base : this->bases()) {
        const auto baseZones = base->zones();
        output.insert(output.end(), baseZones.begin(), baseZones.end());
    }
    return output;
}

std::vector<std::string> Tree::zoneNames() const {
    std::vector<std::string> output;
    for (const auto& zone : this->zones()) {
        output.push_back(zone->name());
    }
    return output;
}

bool Tree::isStructured() const {
    const auto baseList = this->bases();
    return std::all_of(baseList.begin(), baseList.end(), [](const auto& base) {
        return base && base->isStructured();
    });
}

bool Tree::isUnstructured() const {
    const auto baseList = this->bases();
    return std::all_of(baseList.begin(), baseList.end(), [](const auto& base) {
        return base && base->isUnstructured();
    });
}

bool Tree::isHybrid() const {
    return !this->isStructured() && !this->isUnstructured();
}

size_t Tree::numberOfPoints() const {
    size_t total = 0;
    for (const auto& base : this->bases()) {
        total += base->numberOfPoints();
    }
    return total;
}

size_t Tree::numberOfCells() const {
    size_t total = 0;
    for (const auto& base : this->bases()) {
        total += base->numberOfCells();
    }
    return total;
}

size_t Tree::numberOfZones() const {
    return this->zones().size();
}

size_t Tree::numberOfBases() const {
    return this->bases().size();
}

std::vector<std::string> Tree::getElementsTypes() const {
    std::set<std::string> output;
    for (const auto& base : this->bases()) {
        const auto baseTypes = base->getElementsTypes();
        output.insert(baseTypes.begin(), baseTypes.end());
    }
    return std::vector<std::string>(output.begin(), output.end());
}

Tree::ZoneDataList Tree::newFields(
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

Tree::ZoneDataList Tree::fields(
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

Tree::ZoneNamedDataList Tree::allFields(
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

void Tree::assertFieldsSizeCoherency() const {
    for (const auto& base : this->bases()) {
        base->assertFieldsSizeCoherency();
    }
}

void Tree::removeEmptyZones() {
    for (const auto& base : this->bases()) {
        base->removeEmptyZones();
    }
}
