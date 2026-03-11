import numpy as np
import pytest
import importlib

from noder.core import Node
from noder.cgns import Zone


def make_cart():
    x, y, z = np.meshgrid(
        np.linspace(0.0, 1.0, 3),
        np.linspace(0.0, 1.0, 3),
        np.linspace(0.0, 1.0, 3),
        indexing="ij",
    )

    zone = Zone("block")
    coordinates = Node("GridCoordinates", "GridCoordinates_t")

    cx = Node("CoordinateX", "DataArray_t")
    cy = Node("CoordinateY", "DataArray_t")
    cz = Node("CoordinateZ", "DataArray_t")
    cx.set_data(x)
    cy.set_data(y)
    cz.set_data(z)
    coordinates.add_children([cx, cy, cz])

    zone.add_child(coordinates)
    zone.update_shape()
    return zone


def test_zone_init():
    zone = Zone("MyZone")
    assert zone.name() == "MyZone"
    assert zone.type() == "Zone_t"
    assert zone.is_structured()
    assert zone.number_of_points() == 2
    assert zone.number_of_cells() == 1


def test_zone_is_not_exported_from_core_module():
    core = importlib.import_module("noder.core")
    assert not hasattr(core, "Zone")


def test_zone_metrics():
    zone = make_cart()
    assert zone.dim() == 3
    assert zone.shape_of_coordinates() == [3, 3, 3]
    assert zone.number_of_points() == 27
    assert zone.number_of_cells() == 8


def test_new_fields_vertex():
    zone = make_cart()
    field = zone.new_fields("field")
    assert np.array_equal(field, np.zeros((3, 3, 3), dtype=np.float64))


def test_new_fields_cell_center():
    zone = make_cart()
    field = zone.new_fields("field", container="FlowSolution#Centers")
    assert np.array_equal(field, np.zeros((2, 2, 2), dtype=np.float64))


def test_new_fields_dict():
    zone = make_cart()
    f1, f2 = zone.new_fields({"f1": 1.0, "f2": 2.0})
    assert np.array_equal(f1, np.full((3, 3, 3), 1.0))
    assert np.array_equal(f2, np.full((3, 3, 3), 2.0))


def test_fields_and_field():
    zone = make_cart()

    field = zone.field("q")
    assert field.shape == (3, 3, 3)
    assert np.allclose(field, 0.0)

    values = zone.fields(["q", "w"], return_type="dict")
    assert set(values) == {"q", "w"}
    assert values["q"].shape == (3, 3, 3)
    assert values["w"].shape == (3, 3, 3)

    zone.remove_fields("w")
    value = zone.field("w", behavior_if_not_found="pass")
    assert value is None

    with pytest.raises(RuntimeError):
        zone.fields("a", container="MissingContainer", behavior_if_not_found="raise")


def test_xyz_and_all_fields():
    zone = make_cart()
    zone.new_fields("f")

    x, y, z = zone.xyz()
    assert x.shape == (3, 3, 3)
    assert y.shape == (3, 3, 3)
    assert z.shape == (3, 3, 3)

    all_fields = zone.all_fields()
    assert set(all_fields) == {"CoordinateX", "CoordinateY", "CoordinateZ", "f"}

    only_fields = zone.all_fields(include_coordinates=False, return_type="list")
    assert len(only_fields) == 1
    assert only_fields[0].shape == (3, 3, 3)


def test_infer_location_and_coherency():
    zone = make_cart()
    zone.new_fields("v", container="FlowSolution")
    zone.new_fields("c", container="FlowSolution#Centers")

    assert zone.infer_location("FlowSolution") == "Vertex"
    assert zone.infer_location("FlowSolution#Centers") == "CellCenter"

    zone.assert_fields_size_coherency()

    bad_container = Node("BadContainer", "FlowSolution_t")
    location = Node("GridLocation", "GridLocation_t")
    location.set_data("Vertex")
    bad_container.add_child(location)
    bad_field = Node("bad", "DataArray_t")
    bad_field.set_data(np.ones((2,), dtype=np.float64))
    bad_container.add_child(bad_field)
    zone.add_child(bad_container)

    with pytest.raises(RuntimeError):
        zone.assert_fields_size_coherency()


def test_update_shape():
    zone = Zone("Zone")
    with pytest.raises(RuntimeError):
        zone.update_shape()

    fs = Node("FlowSolution", "FlowSolution_t")
    data = Node("Data", "DataArray_t")
    data.set_data(np.ones((3, 2, 5), order="F"))
    fs.add_child(data)
    zone.add_child(fs)

    zone.update_shape()
    assert zone.number_of_points() == 30
    assert zone.number_of_cells() == 8

    data.set_data(np.arange(10))
    zone.update_shape()
    assert zone.number_of_points() == 10
    assert zone.number_of_cells() == 9

    other_data = Node("OtherData", "DataArray_t")
    other_data.set_data(np.ones((3, 9), order="F"))
    fs.add_child(other_data)
    with pytest.raises(RuntimeError):
        zone.update_shape()


def test_is_empty():
    empty_zone = Zone("EmptyZone")
    assert empty_zone.is_empty()

    full_zone = make_cart()
    assert not full_zone.is_empty()


def test_boundaries():
    zone = make_cart()
    zone.new_fields("f")

    boundaries = zone.boundaries()
    assert len(boundaries) == 6
    assert all(isinstance(b, Zone) for b in boundaries)
    assert boundaries[0].dim() == 2
    assert boundaries[0].number_of_points() == 9
