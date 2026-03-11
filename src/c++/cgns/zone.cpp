#include "cgns/zone.hpp"

#include <cctype>
#include <limits>
#include <set>
#include <stdexcept>
#include <unordered_map>

namespace {

const std::vector<std::string> ELEMENTS_TYPES = {
    "Null", "UserDefined",
    "NODE", "BAR_2", "BAR_3", "TRI_3", "TRI_6", "QUAD_4", "QUAD_8", "QUAD_9",
    "TETRA_4", "TETRA_10", "PYRA_5", "PYRA_14", "PENTA_6", "PENTA_15",
    "PENTA_18", "HEXA_8", "HEXA_20", "HEXA_27", "MIXED", "PYRA_13",
    "NGON_n", "NFACE_n", "BAR_4", "TRI_9", "TRI_10", "QUAD_12", "QUAD_16",
    "TETRA_16", "TETRA_20", "PYRA_21", "PYRA_29", "PYRA_30", "PENTA_24",
    "PENTA_38", "PENTA_40", "HEXA_32", "HEXA_56", "HEXA_64", "BAR_5", "TRI_12",
    "TRI_15", "QUAD_P4_16", "QUAD_25", "TETRA_22", "TETRA_34", "TETRA_35",
    "PYRA_P4_29", "PYRA_50", "PYRA_55", "PENTA_33", "PENTA_66", "PENTA_75",
    "HEXA_44", "HEXA_98", "HEXA_125"
};

std::string toLower(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

std::shared_ptr<Data> requireData(const std::shared_ptr<Data>& data, const char* context) {
    if (!data) {
        throw std::runtime_error(std::string(context) + ": data pointer is null");
    }
    return data;
}

std::shared_ptr<Data> maybeRavelData(const std::shared_ptr<Data>& data, bool ravel, const char* context) {
    auto safeData = requireData(data, context);
    if (!ravel || safeData->isNone()) {
        return safeData;
    }
    return safeData->ravel("K");
}

size_t productOfColumn(
    const std::vector<std::array<int64_t, 3>>& entries,
    size_t column,
    const char* context) {

    size_t product = 1;
    for (const auto& entry : entries) {
        if (entry[column] < 0) {
            throw std::runtime_error(std::string(context) + ": zone value contains negative dimensions");
        }

        const size_t value = static_cast<size_t>(entry[column]);
        if (value != 0 && product > std::numeric_limits<size_t>::max() / value) {
            throw std::runtime_error(std::string(context) + ": overflow while multiplying dimensions");
        }
        product *= value;
    }
    return product;
}


std::vector<std::shared_ptr<Node>> coordinateNodes(const Zone& zone) {
    auto grid = const_cast<Zone&>(zone).pick().childByName("GridCoordinates");
    if (!grid) {
        throw std::runtime_error("coordinates: GridCoordinates container not found");
    }

    auto x = grid->pick().childByName("CoordinateX");
    auto y = grid->pick().childByName("CoordinateY");
    auto z = grid->pick().childByName("CoordinateZ");
    if (x && y && z) {
        return {x, y, z};
    }

    throw std::runtime_error("coordinates: expected CoordinateX/CoordinateY/CoordinateZ");
}

std::shared_ptr<Data> sliceBoundaryData(
    const std::shared_ptr<Data>& data,
    size_t axis,
    bool useMin,
    const char* context) {

    auto safeData = requireData(data, context);
    if (safeData->dimensions() == 0) {
        return safeData->clone();
    }
    if (axis >= safeData->dimensions()) {
        throw std::runtime_error(std::string(context) + ": axis out of bounds");
    }

    const int64_t index = useMin ? 0 : -1;
    return safeData->take(index, axis);
}

} // namespace

Zone::Zone(const std::string& name) : Node(name.empty() ? "Zone" : name, "Zone_t") {
    this->setType("Zone_t");
    try {
        this->updateShape();
    } catch (...) {
        auto shapeData = this->data().full({1, 3}, 0.0, "int32");
        shapeData->setItemFromInt64({0, 0}, 2);
        shapeData->setItemFromInt64({0, 1}, 1);
        shapeData->setItemFromInt64({0, 2}, 0);
        this->setData(shapeData);
    }
}

std::shared_ptr<Node> Zone::ensureZoneTypeNode() {
    auto zoneType = this->pick().childByName("ZoneType");
    if (zoneType) {
        return zoneType;
    }

    auto thisPtr = std::const_pointer_cast<Node>(this->selfPtr());
    if (!thisPtr) {
        return nullptr;
    }

    zoneType = std::make_shared<Node>("ZoneType", "ZoneType_t");
    zoneType->setData("Structured");
    zoneType->attachTo(thisPtr);
    return zoneType;
}

std::shared_ptr<Node> Zone::zoneTypeNode() const {
    auto& nav = const_cast<Zone*>(this)->pick();
    auto zoneType = nav.childByName("ZoneType");
    if (zoneType) {
        return zoneType;
    }

    return const_cast<Zone*>(this)->ensureZoneTypeNode();
}

std::shared_ptr<Node> Zone::containerNode(const std::string& containerName) const {
    for (const auto& child : this->children()) {
        if (child && child->name() == containerName) {
            return child;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Zone::directChildrenByType(const std::string& nodeType) const {
    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto& child : this->children()) {
        if (child && child->type() == nodeType) {
            nodes.push_back(child);
        }
    }
    return nodes;
}

std::string Zone::defaultGridLocationFor(const std::string& containerName) {
    static const std::unordered_map<std::string, std::string> mapping = {
        {"FlowSolution", "Vertex"},
        {"FlowSolution#Centers", "CellCenter"},
        {"FlowSolution#Height", "Vertex"},
        {"FlowSolution#EndOfRun", "CellCenter"},
        {"FlowSolution#Init", "CellCenter"},
        {"FlowSolution#SourceTerm", "CellCenter"},
        {"FlowSolution#EndOfRun#Coords", "Vertex"}
    };

    auto it = mapping.find(containerName);
    if (it != mapping.end()) {
        return it->second;
    }
    return "Vertex";
}

std::vector<std::array<int64_t, 3>> Zone::extractZoneShapeEntries(const char* context) const {
    auto shapeData = requireData(this->dataPtr(), context);
    if (shapeData->isNone()) {
        throw std::runtime_error(std::string(context) + ": zone value is none-like");
    }

    if (shapeData->dimensions() != 2) {
        throw std::runtime_error(std::string(context) + ": zone value must be 2D");
    }

    const auto dataShape = shapeData->shape();
    if (dataShape.size() != 2 || dataShape[1] < 2) {
        throw std::runtime_error(std::string(context) + ": zone value must have at least 2 columns");
    }

    std::vector<std::array<int64_t, 3>> entries(dataShape[0]);
    for (size_t row = 0; row < dataShape[0]; ++row) {
        entries[row][0] = shapeData->itemAsInt64({row, 0});
        entries[row][1] = shapeData->itemAsInt64({row, 1});
        entries[row][2] = (dataShape[1] > 2)
            ? shapeData->itemAsInt64({row, 2})
            : 0;
    }
    return entries;
}

std::vector<size_t> Zone::shapeAtLocation(const std::string& location, const char* context) const {
    const auto entries = this->extractZoneShapeEntries(context);
    const size_t column = (location == "CellCenter") ? 1 : 0;

    std::vector<size_t> shape;
    shape.reserve(entries.size());
    for (const auto& entry : entries) {
        if (entry[column] < 0) {
            throw std::runtime_error(std::string(context) + ": negative shape component");
        }
        shape.push_back(static_cast<size_t>(entry[column]));
    }
    return shape;
}

bool Zone::isStructured() const {
    auto zoneType = this->zoneTypeNode();
    if (!zoneType || zoneType->noData() || !zoneType->data().hasString()) {
        return false;
    }
    return zoneType->data().extractString() == "Structured";
}

bool Zone::isUnstructured() const {
    auto zoneType = this->zoneTypeNode();
    if (!zoneType || zoneType->noData() || !zoneType->data().hasString()) {
        return false;
    }
    return zoneType->data().extractString() == "Unstructured";
}

std::vector<std::string> Zone::getElementsTypes() const {
    std::set<std::string> output;

    auto elementsNodes = this->directChildrenByType("Elements_t");
    if (elementsNodes.empty()) {
        if (this->isStructured()) {
            output.insert("STRUCTURED");
        }
    } else {
        for (const auto& elementsNode : elementsNodes) {
            if (!elementsNode || !elementsNode->dataPtr() || elementsNode->dataPtr()->isNone()) {
                continue;
            }

            auto data = requireData(elementsNode->dataPtr(), "getElementsTypes");
            if (data->size() == 0) {
                continue;
            }

            const std::vector<size_t> firstIndex(data->dimensions(), 0);
            const int64_t enumValue = data->itemAsInt64(firstIndex);

            if (enumValue >= 0 && static_cast<size_t>(enumValue) < ELEMENTS_TYPES.size()) {
                output.insert(ELEMENTS_TYPES[static_cast<size_t>(enumValue)]);
            }
        }
    }

    return std::vector<std::string>(output.begin(), output.end());
}

size_t Zone::dim() const {
    if (this->isStructured()) {
        try {
            return this->shapeOfCoordinates().size();
        } catch (...) {
            // Fall back to zone value rows.
        }
    }
    const auto entries = this->extractZoneShapeEntries("dim");
    return entries.size();
}

std::vector<size_t> Zone::shape() const {
    return this->shapeOfCoordinates();
}

std::vector<size_t> Zone::shapeOfCoordinates() const {
    const auto coord = this->pick().byNameGlob("Coordinate*");
    if (!coord) throw std::runtime_error(std::string("shapeOfCoordinates: zone ")+this->path()+std::string(" did not have coordinates"));
    const auto data = requireData(coord->dataPtr(), "shape");
    return data->shape();
}

size_t Zone::numberOfPoints() const {
    const auto entries = this->extractZoneShapeEntries("numberOfPoints");
    return productOfColumn(entries, 0, "numberOfPoints");
}

size_t Zone::numberOfCells() const {
    const auto entries = this->extractZoneShapeEntries("numberOfCells");
    return productOfColumn(entries, 1, "numberOfCells");
}

Zone::DataList Zone::newFields(
    const std::vector<std::pair<std::string, double>>& inputFields,
    const std::string& container,
    const std::string& gridLocation,
    const std::string& dtype,
    bool ravel) {

    std::string resolvedLocation = gridLocation;
    if (resolvedLocation == "auto") {
        resolvedLocation = defaultGridLocationFor(container);
    }

    auto flowSolution = this->containerNode(container);
    if (!flowSolution) {
        auto thisPtr = std::const_pointer_cast<Node>(this->selfPtr());
        if (!thisPtr) {
            throw std::runtime_error("newFields: stack-allocated zones are not supported");
        }

        flowSolution = std::make_shared<Node>(container, "FlowSolution_t");
        flowSolution->attachTo(thisPtr);

        auto locationNode = std::make_shared<Node>("GridLocation", "GridLocation_t");
        locationNode->setData(resolvedLocation);
        locationNode->attachTo(flowSolution);
    }

    const std::string existingLocation = this->inferLocation(container);
    if (resolvedLocation != existingLocation) {
        throw std::invalid_argument(
            "newFields: requested GridLocation '" + resolvedLocation +
            "' at container '" + container +
            "' but existing fields are located at '" + existingLocation + "'");
    }

    std::vector<size_t> targetShape;
    if (existingLocation == "Vertex") {
        targetShape = this->shapeAtLocation("Vertex", "newFields");
    } else if (existingLocation == "CellCenter") {
        targetShape = this->shapeAtLocation("CellCenter", "newFields");
    } else {
        throw std::invalid_argument("newFields: unsupported GridLocation '" + existingLocation + "'");
    }

    DataList outputs;
    outputs.reserve(inputFields.size());
    auto dataPrototype = requireData(this->dataPtr(), "newFields");

    for (const auto& [fieldName, fieldValue] : inputFields) {
        auto fieldNode = flowSolution->pick().childByName(fieldName);
        if (!fieldNode) {
            fieldNode = std::make_shared<Node>(fieldName, "DataArray_t");
            fieldNode->attachTo(flowSolution);
        } else {
            fieldNode->setType("DataArray_t");
        }

        auto data = dataPrototype->full(targetShape, fieldValue, dtype);
        fieldNode->setData(maybeRavelData(data, ravel, "newFields"));
        outputs.push_back(fieldNode->dataPtr());
    }

    return outputs;
}

void Zone::removeFields(const std::vector<std::string>& fieldNames, const std::string& container) {
    auto flowSolution = this->containerNode(container);
    if (!flowSolution) {
        return;
    }

    for (const auto& fieldName : fieldNames) {
        auto fieldNode = flowSolution->pick().childByName(fieldName);
        if (fieldNode) {
            fieldNode->detach();
        }
    }
}

Zone::DataList Zone::fields(
    const std::vector<std::string>& fieldNames,
    const std::string& container,
    const std::string& behaviorIfNotFound,
    const std::string& dtype,
    bool ravel) {

    if (fieldNames.empty()) {
        return {};
    }

    auto flowSolution = this->containerNode(container);
    if (!flowSolution) {
        if (behaviorIfNotFound == "raise") {
            throw std::runtime_error("fields: container '" + container + "' not found in " + this->name());
        }
        if (behaviorIfNotFound == "create") {
            std::vector<std::pair<std::string, double>> newFieldSpecs;
            newFieldSpecs.reserve(fieldNames.size());
            for (const auto& fieldName : fieldNames) {
                newFieldSpecs.emplace_back(fieldName, 0.0);
            }
            return this->newFields(newFieldSpecs, container, "auto", dtype, ravel);
        }
        if (behaviorIfNotFound == "pass") {
            return {};
        }
        throw std::invalid_argument("fields: unsupported BehaviorIfNotFound '" + behaviorIfNotFound + "'");
    }

    DataList outputs;
    outputs.reserve(fieldNames.size());

    for (const auto& fieldName : fieldNames) {
        auto fieldNode = flowSolution->pick().childByName(fieldName);
        if (!fieldNode) {
            if (behaviorIfNotFound == "create") {
                const auto created = this->newFields({{fieldName, 0.0}}, container, "auto", dtype, ravel);
                outputs.push_back(created.empty() ? nullptr : created.front());
            } else if (behaviorIfNotFound == "raise") {
                throw std::runtime_error(
                    "fields: '" + fieldName + "' not found in container " + flowSolution->path());
            } else if (behaviorIfNotFound == "pass") {
                outputs.push_back(nullptr);
            } else {
                throw std::invalid_argument("fields: unsupported BehaviorIfNotFound '" + behaviorIfNotFound + "'");
            }
            continue;
        }

        outputs.push_back(maybeRavelData(fieldNode->dataPtr(), ravel, "fields"));
    }

    return outputs;
}

std::shared_ptr<Data> Zone::field(
    const std::string& fieldName,
    const std::string& container,
    const std::string& behaviorIfNotFound,
    const std::string& dtype,
    bool ravel) {

    const auto output = this->fields({fieldName}, container, behaviorIfNotFound, dtype, ravel);
    if (output.empty()) {
        return nullptr;
    }
    return output.front();
}

Zone::DataList Zone::xyz(bool ravel) const {
    const auto coords = coordinateNodes(*this);
    return {
        maybeRavelData(coords[0]->dataPtr(), ravel, "xyz"),
        maybeRavelData(coords[1]->dataPtr(), ravel, "xyz"),
        maybeRavelData(coords[2]->dataPtr(), ravel, "xyz")
    };
}

Zone::DataList Zone::xy(bool ravel) const {
    const auto coords = this->xyz(ravel);
    return {coords[0], coords[1]};
}

Zone::DataList Zone::xz(bool ravel) const {
    const auto coords = this->xyz(ravel);
    return {coords[0], coords[2]};
}

Zone::DataList Zone::yz(bool ravel) const {
    const auto coords = this->xyz(ravel);
    return {coords[1], coords[2]};
}

std::shared_ptr<Data> Zone::x(bool ravel) const {
    return this->xyz(ravel)[0];
}

std::shared_ptr<Data> Zone::y(bool ravel) const {
    return this->xyz(ravel)[1];
}

std::shared_ptr<Data> Zone::z(bool ravel) const {
    return this->xyz(ravel)[2];
}

Zone::NamedDataList Zone::allFields(
    bool includeCoordinates,
    bool appendContainerToFieldName,
    bool ravel) const {

    NamedDataList output;

    if (includeCoordinates) {
        const auto coords = this->xyz(ravel);
        output.emplace_back("CoordinateX", coords[0]);
        output.emplace_back("CoordinateY", coords[1]);
        output.emplace_back("CoordinateZ", coords[2]);
    }

    for (const auto& flowSolution : this->directChildrenByType("FlowSolution_t")) {
        for (const auto& fieldNode : flowSolution->pick().allByType("DataArray_t", 2)) {
            const std::string key = appendContainerToFieldName
                ? (flowSolution->name() + "/" + fieldNode->name())
                : fieldNode->name();

            output.emplace_back(key, maybeRavelData(fieldNode->dataPtr(), ravel, "allFields"));
        }
    }

    return output;
}

void Zone::assertFieldsSizeCoherency() const {
    const size_t zonePoints = this->numberOfPoints();
    const size_t zoneCells = this->numberOfCells();

    for (const auto& container : this->directChildrenByType("FlowSolution_t")) {
        const auto fieldsInContainer = container->pick().allByType("DataArray_t", 2);
        if (fieldsInContainer.empty()) {
            continue;
        }

        const size_t expectedSize = requireData(
            fieldsInContainer.front()->dataPtr(),
            "assertFieldsSizeCoherency")->size();
        for (const auto& fieldNode : fieldsInContainer) {
            const size_t fieldSize = requireData(fieldNode->dataPtr(), "assertFieldsSizeCoherency")->size();
            if (fieldSize != expectedSize) {
                throw std::runtime_error(
                    "assertFieldsSizeCoherency: not all fields have same size in " + container->path());
            }
        }

        auto gridLocationNode = container->pick().childByName("GridLocation");
        if (gridLocationNode && !gridLocationNode->noData() && gridLocationNode->data().hasString()) {
            const std::string location = gridLocationNode->data().extractString();
            if (location == "Vertex") {
                if (expectedSize != zonePoints) {
                    throw std::runtime_error(
                        "assertFieldsSizeCoherency: mismatch between zone points and field size at " +
                        container->path());
                }
            } else if (location == "CellCenter") {
                if (expectedSize != zoneCells) {
                    throw std::runtime_error(
                        "assertFieldsSizeCoherency: mismatch between zone cells and field size at " +
                        container->path());
                }
            } else {
                throw std::runtime_error(
                    "assertFieldsSizeCoherency: unsupported GridLocation '" + location + "'");
            }
        } else if (expectedSize != zonePoints && expectedSize != zoneCells) {
            throw std::runtime_error(
                "assertFieldsSizeCoherency: field size at " + container->path() +
                " does not correspond to zone points or cells");
        }
    }
}

std::string Zone::inferLocation(const std::string& container) const {
    if (container == "GridCoordinates") {
        return "Vertex";
    }

    auto flowSolution = this->containerNode(container);
    if (!flowSolution) {
        throw std::runtime_error("inferLocation: container '" + container + "' not found in " + this->path());
    }

    auto gridLocationNode = flowSolution->pick().childByName("GridLocation");
    if (gridLocationNode && !gridLocationNode->noData() && gridLocationNode->data().hasString()) {
        return gridLocationNode->data().extractString();
    }

    const auto dataChild = flowSolution->pick().childByType("DataArray_t");
    if (!dataChild) {
        return defaultGridLocationFor(container);
    }

    const size_t fieldSize = requireData(dataChild->dataPtr(), "inferLocation")->size();
    if (fieldSize == this->numberOfPoints()) {
        return "Vertex";
    }
    if (fieldSize == this->numberOfCells()) {
        return "CellCenter";
    }

    throw std::runtime_error(
        "inferLocation: field size did not match zone points/cells in container " + flowSolution->path());
}

bool Zone::hasFields() const {
    return !this->directChildrenByType("FlowSolution_t").empty();
}

Zone::ShapePair Zone::getArrayShapes() const {
    std::vector<size_t> shapeVertex;
    std::vector<size_t> shapeCellCenter;

    const auto updateShape = [](std::vector<size_t>& target, const std::vector<size_t>& candidate, const std::string& where) {
        if (candidate.empty()) {
            return;
        }
        if (target.empty()) {
            target = candidate;
            return;
        }
        if (target != candidate) {
            throw std::runtime_error(
                "getArrayShapes: inconsistent shape in " + where);
        }
    };

    auto gridCoordinates = this->pick().childByName("GridCoordinates");
    if (gridCoordinates) {
        for (const auto& dataNode : gridCoordinates->pick().allByType("DataArray_t", 2)) {
            updateShape(shapeVertex, requireData(dataNode->dataPtr(), "getArrayShapes")->shape(), dataNode->path());
        }
    }

    for (const auto& flowSolution : this->directChildrenByType("FlowSolution_t")) {
        std::string location;
        auto locationNode = flowSolution->pick().childByName("GridLocation");
        if (locationNode && !locationNode->noData() && locationNode->data().hasString()) {
            location = locationNode->data().extractString();
        } else {
            location = defaultGridLocationFor(flowSolution->name());
        }

        for (const auto& dataNode : flowSolution->pick().allByType("DataArray_t", 2)) {
            const auto candidate = requireData(dataNode->dataPtr(), "getArrayShapes")->shape();
            if (location == "Vertex") {
                updateShape(shapeVertex, candidate, dataNode->path());
            } else if (location == "CellCenter") {
                updateShape(shapeCellCenter, candidate, dataNode->path());
            } else {
                throw std::runtime_error(
                    "getArrayShapes: unsupported GridLocation '" + location + "' at " + flowSolution->path());
            }
        }
    }

    if (shapeVertex.empty() && shapeCellCenter.empty()) {
        throw std::runtime_error("getArrayShapes: zone has no DataArray nodes");
    }

    if (shapeCellCenter.empty()) {
        shapeCellCenter = shapeVertex;
        for (auto& value : shapeCellCenter) {
            if (value == 0) {
                throw std::runtime_error("getArrayShapes: cannot infer CellCenter shape from zero-sized Vertex shape");
            }
            value -= 1;
        }
    } else if (shapeVertex.empty()) {
        shapeVertex = shapeCellCenter;
        for (auto& value : shapeVertex) {
            value += 1;
        }
    }

    return {shapeVertex, shapeCellCenter};
}

void Zone::updateShape() {
    const auto [shapeVertex, unusedShapeCellCenter] = this->getArrayShapes();
    (void)unusedShapeCellCenter;

    auto shapeData = this->data().full({shapeVertex.size(), 3}, 0.0, "int32");

    for (size_t i = 0; i < shapeVertex.size(); ++i) {
        if (shapeVertex[i] > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
            throw std::runtime_error("updateShape: dimension exceeds int32_t range");
        }

        const int64_t value = static_cast<int64_t>(shapeVertex[i]);
        shapeData->setItemFromInt64({i, 0}, value);
        shapeData->setItemFromInt64({i, 1}, (value > 0) ? (value - 1) : 0);
        shapeData->setItemFromInt64({i, 2}, 0);
    }

    this->setData(shapeData);
}

bool Zone::isEmpty() const {
    auto gridCoordinates = const_cast<Zone*>(this)->pick().childByName("GridCoordinates");
    if (!gridCoordinates) {
        return true;
    }

    auto firstCoordinate = gridCoordinates->pick().childByType("DataArray_t");
    if (!firstCoordinate) {
        return true;
    }

    return !firstCoordinate->dataPtr() || firstCoordinate->dataPtr()->isNone();
}

Zone::ZoneList Zone::boundaries() {
    if (!this->isStructured()) {
        throw std::runtime_error("boundaries: only implemented for structured zones");
    }

    const size_t zoneDim = this->dim();
    if (zoneDim == 3) return {this->imin(), this->imax(), this->jmin(), this->jmax(), this->kmin(), this->kmax()};
    if (zoneDim == 2) return {this->imin(), this->imax(), this->jmin(), this->jmax()};
    return {this->imin(), this->imax()};
}

std::shared_ptr<Zone> Zone::boundary(const std::string& index, const std::string& bound) {
    if (!this->isStructured()) {
        throw std::runtime_error("boundary: only meaningful for structured zones");
    }

    const std::string lowerIndex = toLower(index);
    const std::string lowerBound = toLower(bound);

    size_t axis = 0;
    if (lowerIndex == "i") axis = 0;
    else if (lowerIndex == "j") axis = 1;
    else if (lowerIndex == "k") axis = 2;
    else throw std::invalid_argument("boundary: index must be one of i, j, k");

    const bool useMin = (lowerBound == "min");
    if (!useMin && lowerBound != "max") {
        throw std::invalid_argument("boundary: bound must be min or max");
    }

    const size_t zoneDim = this->dim();
    if (axis >= zoneDim) {
        throw std::runtime_error("boundary: requested axis is not available in this zone dimension");
    }

    const auto coords = coordinateNodes(*this);

    auto output = std::make_shared<Zone>(this->name() + "_" + lowerIndex + lowerBound);
    auto outputNode = std::const_pointer_cast<Node>(output->selfPtr());
    if (!outputNode) {
        throw std::runtime_error("boundary: could not create output zone owner pointer");
    }

    auto gridCoordinates = std::make_shared<Node>("GridCoordinates", "GridCoordinates_t");
    gridCoordinates->attachTo(outputNode);

    const std::array<std::string, 3> coordinateNames = {"CoordinateX", "CoordinateY", "CoordinateZ"};
    for (size_t i = 0; i < coordinateNames.size(); ++i) {
        auto coord = std::make_shared<Node>(coordinateNames[i], "DataArray_t");
        coord->setData(sliceBoundaryData(coords[i]->dataPtr(), axis, useMin, "boundary"));
        coord->attachTo(gridCoordinates);
    }

    for (const auto& flowSolution : this->directChildrenByType("FlowSolution_t")) {
        auto outputFlowSolution = std::make_shared<Node>(flowSolution->name(), "FlowSolution_t");
        outputFlowSolution->attachTo(outputNode);

        auto locationNode = flowSolution->pick().childByName("GridLocation");
        if (locationNode) {
            auto copiedLocation = std::make_shared<Node>("GridLocation", "GridLocation_t");
            if (!locationNode->noData()) {
                copiedLocation->setData(locationNode->data());
            }
            copiedLocation->attachTo(outputFlowSolution);
        }

        for (const auto& fieldNode : flowSolution->pick().allByType("DataArray_t", 2)) {
            auto outputField = std::make_shared<Node>(fieldNode->name(), "DataArray_t");
            outputField->setData(sliceBoundaryData(fieldNode->dataPtr(), axis, useMin, "boundary"));
            outputField->attachTo(outputFlowSolution);
        }
    }

    output->updateShape();
    output->ensureZoneTypeNode();
    return output;
}

std::shared_ptr<Zone> Zone::imin() { return this->boundary("i", "min"); }
std::shared_ptr<Zone> Zone::imax() { return this->boundary("i", "max"); }
std::shared_ptr<Zone> Zone::jmin() { return this->boundary("j", "min"); }
std::shared_ptr<Zone> Zone::jmax() { return this->boundary("j", "max"); }
std::shared_ptr<Zone> Zone::kmin() { return this->boundary("k", "min"); }
std::shared_ptr<Zone> Zone::kmax() { return this->boundary("k", "max"); }
