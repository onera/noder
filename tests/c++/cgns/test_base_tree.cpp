#include "test_base_tree.hpp"

#include "array/array.hpp"
#include "array/array_numpy_bridge.hpp"
#include "cgns/base.hpp"
#include "cgns/tree.hpp"
#include "cgns/zone.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace py = pybind11;

namespace {

std::shared_ptr<Zone> makeCartesianZone(
    const std::string& name,
    const std::vector<ssize_t>& shape) {

    auto zone = std::make_shared<Zone>(name);
    auto zoneNode = std::const_pointer_cast<Node>(zone->selfPtr());
    if (!zoneNode) {
        throw py::value_error("makeCartesianZone: expected heap-managed zone");
    }

    auto coordinates = std::make_shared<Node>("GridCoordinates", "GridCoordinates_t");
    coordinates->attachTo(zoneNode);

    auto addCoordinate = [&](const std::string& coordName) {
        py::array_t<double> values(shape);
        auto coord = std::make_shared<Node>(coordName, "DataArray_t");
        coord->setData(std::make_shared<Array>(arraybridge::arrayFromPyArray(values)));
        coord->attachTo(coordinates);
    };

    addCoordinate("CoordinateX");
    addCoordinate("CoordinateY");
    addCoordinate("CoordinateZ");

    zone->updateShape();
    return zone;
}

} // namespace

void test_base_new_base_dimensions() {
    auto zoneA = makeCartesianZone("ZoneA", {3, 3, 3});
    auto zoneB = makeCartesianZone("ZoneB", {4, 3, 2});

    auto base = newBase("Base", {zoneA, zoneB});
    if (base->name() != "Base") throw py::value_error("expected base name Base");
    if (base->type() != "CGNSBase_t") throw py::value_error("expected CGNSBase_t");
    if (base->dim() != 3) throw py::value_error("expected cell dimension 3");
    if (base->physicalDimension() != 3) throw py::value_error("expected physical dimension 3");
    if (base->numberOfZones() != 2) throw py::value_error("expected two zones");
    if (zoneA->parent().lock().get() != base.get()) throw py::value_error("zone should be attached to base");
}

void test_base_aggregates_zones() {
    auto zoneA = makeCartesianZone("ZoneA", {3, 3, 3});
    auto zoneB = makeCartesianZone("ZoneB", {4, 3, 2});
    auto base = newBase("Base", {zoneA, zoneB});

    if (!base->isStructured()) throw py::value_error("base should be structured");
    if (base->numberOfPoints() != zoneA->numberOfPoints() + zoneB->numberOfPoints()) {
        throw py::value_error("base point count mismatch");
    }
    if (base->numberOfCells() != zoneA->numberOfCells() + zoneB->numberOfCells()) {
        throw py::value_error("base cell count mismatch");
    }

    auto fields = base->newFields({{"q", 7.0}});
    if (fields.size() != 2) throw py::value_error("expected field output for two zones");
    if (zoneA->field("q")->size() != zoneA->numberOfPoints()) {
        throw py::value_error("zoneA field size mismatch");
    }
}

void test_tree_aggregates_bases_and_zones() {
    auto zoneA = makeCartesianZone("ZoneA", {3, 3, 3});
    auto zoneB = makeCartesianZone("ZoneB", {4, 3, 2});
    auto base = newBase("Base", {zoneA});

    auto tree = std::make_shared<Tree>();
    tree->add(base);
    tree->add(zoneB);

    if (tree->type() != "CGNSTree_t") throw py::value_error("expected CGNSTree_t");
    if (tree->numberOfBases() != 1) throw py::value_error("expected one base");
    if (tree->numberOfZones() != 2) throw py::value_error("expected two zones");
    if (tree->numberOfPoints() != zoneA->numberOfPoints() + zoneB->numberOfPoints()) {
        throw py::value_error("tree point count mismatch");
    }

    auto names = tree->zoneNames();
    if (names.size() != 2 || names[0] != "ZoneA" || names[1] != "ZoneB") {
        throw py::value_error("unexpected tree zone names");
    }
}

void test_new_base_rejects_incoherent_dimensions() {
    auto zone3D = makeCartesianZone("Zone3D", {3, 3, 3});
    auto zone2D = makeCartesianZone("Zone2D", {3, 3});

    bool raised = false;
    try {
        (void)newBase("BadBase", {zone3D, zone2D});
    } catch (const std::invalid_argument&) {
        raised = true;
    }

    if (!raised) throw py::value_error("newBase should reject mixed cell dimensions");
}

void test_newZoneFromArrays() {
    py::array_t<double> x({3, 2});
    py::array_t<double> y({3, 2});
    py::array_t<double> z({3, 2});
    py::array_t<double> pressure({3, 2});

    auto zone = newZoneFromArrays(
        "Block",
        {"x", "y", "z", "Pressure"},
        {
            std::make_shared<Array>(arraybridge::arrayFromPyArray(x)),
            std::make_shared<Array>(arraybridge::arrayFromPyArray(y)),
            std::make_shared<Array>(arraybridge::arrayFromPyArray(z)),
            std::make_shared<Array>(arraybridge::arrayFromPyArray(pressure))
        });

    if (zone->name() != "Block") throw py::value_error("expected zone name Block");
    if (zone->type() != "Zone_t") throw py::value_error("expected Zone_t");
    if (zone->dim() != 2) throw py::value_error("expected 2D zone");
    if (zone->numberOfPoints() != 6) throw py::value_error("expected six points");
    if (zone->numberOfCells() != 2) throw py::value_error("expected two cells");

    auto coordinates = zone->pick().childByName("GridCoordinates");
    if (!coordinates) throw py::value_error("expected GridCoordinates");
    if (!coordinates->pick().childByName("CoordinateX")) throw py::value_error("expected CoordinateX");
    if (!coordinates->pick().childByName("CoordinateY")) throw py::value_error("expected CoordinateY");
    if (!coordinates->pick().childByName("CoordinateZ")) throw py::value_error("expected CoordinateZ");

    auto fields = zone->pick().childByName("FlowSolution");
    if (!fields) throw py::value_error("expected FlowSolution");
    auto pressureNode = fields->pick().childByName("Pressure");
    if (!pressureNode) throw py::value_error("expected Pressure field");
    if (pressureNode->dataPtr()->shape() != std::vector<size_t>({3, 2})) {
        throw py::value_error("unexpected Pressure shape");
    }
}
