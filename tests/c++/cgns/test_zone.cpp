#include "test_zone.hpp"

#include "array/array.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;

namespace {

std::shared_ptr<Array> arrayFromData(const std::shared_ptr<Data>& data, const std::string& context) {
    auto arrayData = std::dynamic_pointer_cast<Array>(data);
    if (!arrayData) {
        throw py::value_error(context + ": data is not an Array");
    }
    return arrayData;
}

void assertShape(const std::vector<size_t>& got, const std::vector<size_t>& expected, const std::string& context) {
    if (got != expected) {
        throw py::value_error(context + ": shape mismatch");
    }
}

std::shared_ptr<Zone> makeCartesianZone(
    const std::string& name = "Zone",
    size_t ni = 3,
    size_t nj = 3,
    size_t nk = 3) {

    auto zone = std::make_shared<Zone>(name);
    auto zoneNode = std::const_pointer_cast<Node>(zone->selfPtr());
    if (!zoneNode) {
        throw py::value_error("makeCartesianZone: expected heap-managed zone");
    }

    auto coordinates = std::make_shared<Node>("GridCoordinates", "GridCoordinates_t");
    coordinates->attachTo(zoneNode);

    const std::vector<ssize_t> shape = {
        static_cast<ssize_t>(ni),
        static_cast<ssize_t>(nj),
        static_cast<ssize_t>(nk)};

    py::array_t<double> x(shape);
    py::array_t<double> y(shape);
    py::array_t<double> z(shape);

    auto xAcc = x.mutable_unchecked<3>();
    auto yAcc = y.mutable_unchecked<3>();
    auto zAcc = z.mutable_unchecked<3>();
    for (size_t i = 0; i < ni; ++i) {
        for (size_t j = 0; j < nj; ++j) {
            for (size_t k = 0; k < nk; ++k) {
                const ssize_t ii = static_cast<ssize_t>(i);
                const ssize_t jj = static_cast<ssize_t>(j);
                const ssize_t kk = static_cast<ssize_t>(k);
                xAcc(ii, jj, kk) = static_cast<double>(i);
                yAcc(ii, jj, kk) = static_cast<double>(j);
                zAcc(ii, jj, kk) = static_cast<double>(k);
            }
        }
    }

    auto addCoordinate = [&](const std::string& coordName, const py::array_t<double>& values) {
        auto coord = std::make_shared<Node>(coordName, "DataArray_t");
        coord->setData(std::make_shared<Array>(values));
        coord->attachTo(coordinates);
    };

    addCoordinate("CoordinateX", x);
    addCoordinate("CoordinateY", y);
    addCoordinate("CoordinateZ", z);

    zone->updateShape();
    return zone;
}

} // namespace

void test_zone_init() {
    auto zone = std::make_shared<Zone>("MyZone");

    if (zone->name() != "MyZone") throw py::value_error("expected zone name MyZone");
    if (zone->type() != "Zone_t") throw py::value_error("expected zone type Zone_t");
    if (!zone->isStructured()) throw py::value_error("expected zone to be structured by default");
    if (zone->numberOfPoints() != 2) throw py::value_error("expected default zone points = 2");
    if (zone->numberOfCells() != 1) throw py::value_error("expected default zone cells = 1");
}

void test_zone_structure_flags() {
    auto zone = std::make_shared<Zone>("MyZone");
    if (!zone->isStructured()) throw py::value_error("zone should start structured");

    auto zoneType = zone->pick().childByName("ZoneType");
    if (!zoneType) throw py::value_error("ZoneType node was not created");

    zoneType->setData("Unstructured");
    if (!zone->isUnstructured()) throw py::value_error("zone should be unstructured after ZoneType update");
    if (zone->isStructured()) throw py::value_error("structured flag should be false when unstructured");
}

void test_zone_metrics() {
    auto zone = makeCartesianZone();

    if (zone->dim() != 3) throw py::value_error("expected 3D zone");
    assertShape(zone->shapeOfCoordinates(), {3, 3, 3}, "shape");
    if (zone->numberOfPoints() != 27) throw py::value_error("expected 27 points");
    if (zone->numberOfCells() != 8) throw py::value_error("expected 8 cells");
}

void test_zone_new_fields() {
    auto zone = makeCartesianZone();

    auto vertexFields = zone->newFields({{"field", 0.0}});
    if (vertexFields.size() != 1) throw py::value_error("expected one created vertex field");
    assertShape(arrayFromData(vertexFields[0], "newFields vertex")->shape(), {3, 3, 3}, "newFields vertex");

    auto centerFields = zone->newFields({{"center", 2.0}}, "FlowSolution#Centers");
    if (centerFields.size() != 1) throw py::value_error("expected one created center field");
    auto centerArray = arrayFromData(centerFields[0], "newFields center");
    assertShape(centerArray->shape(), {2, 2, 2}, "newFields center");
    if (centerArray->getItemAtIndex<double>(0) != 2.0) {
        throw py::value_error("expected center field value 2.0");
    }
}

void test_zone_fields_accessors() {
    auto zone = makeCartesianZone();

    auto q = zone->field("q");
    assertShape(arrayFromData(q, "field q")->shape(), {3, 3, 3}, "field q");

    auto values = zone->fields({"q", "w"});
    if (values.size() != 2) throw py::value_error("expected two fields");
    assertShape(arrayFromData(values[1], "field w")->shape(), {3, 3, 3}, "field w");

    zone->removeFields({"w"});
    auto missing = zone->fields({"w"}, "FlowSolution", "pass");
    if (missing.size() != 1 || missing[0] != nullptr) {
        throw py::value_error("expected missing field to return nullptr in pass mode");
    }

    bool raised = false;
    try {
        (void)zone->fields({"not_here"}, "MissingContainer", "raise");
    } catch (const std::exception&) {
        raised = true;
    }
    if (!raised) throw py::value_error("expected raise behavior for missing container");
}

void test_zone_all_fields_and_xyz() {
    auto zone = makeCartesianZone();
    zone->newFields({{"f", 1.0}});

    auto xyz = zone->xyz();
    if (xyz.size() != 3) throw py::value_error("xyz should return 3 arrays");
    assertShape(arrayFromData(xyz[0], "xyz")->shape(), {3, 3, 3}, "xyz x");

    auto all = zone->allFields(true, false, false);
    if (all.size() != 4) throw py::value_error("allFields should include coordinates + one field");
    if (all[0].first != "CoordinateX") throw py::value_error("first field should be CoordinateX");

    auto onlyFields = zone->allFields(false, false, false);
    if (onlyFields.size() != 1 || onlyFields[0].first != "f") {
        throw py::value_error("allFields without coordinates should include only field nodes");
    }
}

void test_zone_infer_location_and_coherency() {
    auto zone = makeCartesianZone();
    zone->newFields({{"vertex_field", 1.0}}, "FlowSolution");
    zone->newFields({{"center_field", 2.0}}, "FlowSolution#Centers");

    if (zone->inferLocation("FlowSolution") != "Vertex") {
        throw py::value_error("expected FlowSolution at Vertex");
    }
    if (zone->inferLocation("FlowSolution#Centers") != "CellCenter") {
        throw py::value_error("expected FlowSolution#Centers at CellCenter");
    }

    zone->assertFieldsSizeCoherency();

    auto zoneNode = std::const_pointer_cast<Node>(zone->selfPtr());
    auto bad = std::make_shared<Node>("BadContainer", "FlowSolution_t");
    bad->attachTo(zoneNode);

    auto badLocation = std::make_shared<Node>("GridLocation", "GridLocation_t");
    badLocation->setData("Vertex");
    badLocation->attachTo(bad);

    py::array_t<double> invalidShape(std::vector<ssize_t>{2});
    auto badField = std::make_shared<Node>("bad", "DataArray_t");
    badField->setData(std::make_shared<Array>(invalidShape));
    badField->attachTo(bad);

    bool raised = false;
    try {
        zone->assertFieldsSizeCoherency();
    } catch (const std::exception&) {
        raised = true;
    }
    if (!raised) throw py::value_error("coherency check should fail for mismatched field size");
}

void test_zone_update_shape() {
    auto zone = std::make_shared<Zone>("Zone");
    auto zoneNode = std::const_pointer_cast<Node>(zone->selfPtr());

    auto flowSolution = std::make_shared<Node>("FlowSolution", "FlowSolution_t");
    flowSolution->attachTo(zoneNode);

    py::array_t<double> values(std::vector<ssize_t>{3, 2, 5});
    auto data = std::make_shared<Node>("Data", "DataArray_t");
    data->setData(std::make_shared<Array>(values));
    data->attachTo(flowSolution);

    zone->updateShape();
    if (zone->numberOfPoints() != 30) throw py::value_error("expected 30 points after updateShape");
    if (zone->numberOfCells() != 8) throw py::value_error("expected 8 cells after updateShape");
}

void test_zone_is_empty() {
    auto emptyZone = std::make_shared<Zone>("EmptyZone");
    if (!emptyZone->isEmpty()) throw py::value_error("zone without GridCoordinates should be empty");

    auto filledZone = makeCartesianZone();
    if (filledZone->isEmpty()) throw py::value_error("zone with coordinates should not be empty");
}

void test_zone_boundaries() {
    auto zone = makeCartesianZone();
    zone->newFields({{"f", 1.0}});

    auto boundaries = zone->boundaries();
    if (boundaries.size() != 6) throw py::value_error("3D zone should produce 6 boundaries");
    if (boundaries[0]->dim() != 2) throw py::value_error("boundary dimension should be 2 for 3D source");
    if (boundaries[0]->numberOfPoints() != 9) throw py::value_error("boundary should have 9 points");

    auto jMax = zone->boundary("j", "max");
    if (jMax->dim() != 2) throw py::value_error("jmax boundary should be 2D");
}
