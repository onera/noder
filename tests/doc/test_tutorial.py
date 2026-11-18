"""Executable examples used by the user tutorial pages.

Code marked with ``docs:start`` and ``docs:end`` is included directly in the
Sphinx pages, so the rendered examples stay tied to the tested code below.
"""

# docs:start imports
import numpy as np

import noder
from noder import (
    new_base,
    new_node,
    new_tree,
    new_zone_from_arrays,
    new_zone_from_dict,
)
# docs:end imports


import pytest

pytestmark = pytest.mark.doc


def test_tutorial_create_tree_and_attach_data():
    # docs:start tree
    # Create a general node tree with the convenience factory.
    simulation = new_node("Simulation", "UserDefinedData_t")
    mesh = new_node("Mesh", "UserDefinedData_t", parent=simulation)
    coordinate_x = new_node(
        "CoordinateX",
        "DataArray_t",
        np.array([0.0, 0.5, 1.0], dtype=np.float64),
        parent=mesh,
    )
    # docs:end tree

    assert mesh.parent() is simulation
    assert coordinate_x.parent() is mesh
    assert simulation.pick().by_name("CoordinateX") is coordinate_x


def test_tutorial_typed_payloads():
    # docs:start payload-types
    fields = new_node("Fields", "UserDefinedData_t")
    payloads = {
        "Iteration": np.array([0, 1], dtype=np.int32),
        "Converged": np.array([False, True], dtype=np.bool_),
        "Pressure": np.array([101325.0, 100980.0], dtype=np.float64),
        "Coefficient": np.array([1.0 + 2.0j, 2.0 - 0.5j], dtype=np.complex64),
    }

    for name, values in payloads.items():
        new_node(name, "DataArray_t", values, parent=fields)
    # docs:end payload-types

    for name, expected in payloads.items():
        field = fields.pick().child_by_name(name)
        assert field is not None
        assert field.data().getPyArray().dtype == expected.dtype


@pytest.mark.doc_example(target="noder.cgns.new_tree")
def test_tutorial_cgns_constructors():
    # docs:start cgns-constructors
    import numpy as np

    from noder import new_base, new_tree, new_zone_from_arrays, new_zone_from_dict

    x, y = np.meshgrid(np.arange(3.0), np.arange(2.0), indexing="ij")
    z = np.zeros_like(x)
    pressure = x + y
    array_names = ["x", "y", "z", "Pressure"]
    arrays = [x, y, z, pressure]

    # Name arrays explicitly, or provide the same information as a mapping.
    wing = new_zone_from_arrays("Wing", array_names, arrays)
    tail = new_zone_from_dict("Tail", dict(zip(array_names, arrays)))
    base = new_base("StandaloneBase", [wing, tail])

    # new_tree is convenient when building named Bases directly from Zones.
    # These are fresh Zones because attaching nodes reparents them.
    tree = new_tree(
        MainBase=[
            new_zone_from_arrays("Wing", array_names, arrays),
            new_zone_from_dict("Tail", dict(zip(array_names, arrays))),
        ]
    )
    # docs:end cgns-constructors

    assert base.number_of_zones() == 2
    assert tree.base_names() == ["MainBase"]
    assert tree.base("MainBase").number_of_zones() == 2


def test_tutorial_search_tree():
    # docs:start search
    simulation = new_node("Simulation", "UserDefinedData_t")
    for zone_name, pressure_values in (
        ("Zone_A", [101325.0, 101110.0]),
        ("Zone_B", [100980.0, 100750.0]),
    ):
        zone = new_node(zone_name, "Zone_t", parent=simulation)
        new_node(
            "Pressure",
            "DataArray_t",
            np.array(pressure_values, dtype=np.float64),
            parent=zone,
        )

    # Exact and glob lookups work well for simple questions.
    first_pressure = simulation.pick().by_name("Pressure")
    zones = simulation.pick().all_by_name_glob("Zone_*")

    # A predicate combines name and type tests in one search expression.
    pressure_nodes = simulation.pick().all_by_predicate(
        "/ n:Pressure & t:DataArray_t"
    )

    # Shortcuts for the combined name/type/data glob search:
    first_by_glob = simulation.get(name="Pressure", type="DataArray_t")
    all_by_glob = simulation.group(name="Pressure", type="DataArray_t")
    # docs:end search

    assert first_pressure is not None
    assert [zone.name() for zone in zones] == ["Zone_A", "Zone_B"]
    assert len(pressure_nodes) == 2
    assert first_by_glob is first_pressure
    assert all_by_glob == pressure_nodes
    assert all(node.type() == "DataArray_t" for node in pressure_nodes)


def test_tutorial_yaml_round_trip():
    # docs:start yaml-round-trip
    from pathlib import Path
    from tempfile import TemporaryDirectory

    root = new_node("Run", "UserDefinedData_t")
    new_node(
        "Pressure",
        "DataArray_t",
        np.array([101325.0, 100980.0], dtype=np.float64),
        parent=root,
    )

    # YAML is convenient for small, inspectable trees and works without HDF5.
    with TemporaryDirectory() as temporary_directory:
        filename = Path(temporary_directory) / "run.yaml"
        root.write(str(filename))
        restored = noder.read(str(filename))

    restored_pressure = restored.pick().by_name("Pressure")
    # docs:end yaml-round-trip

    assert restored_pressure is not None
    np.testing.assert_array_equal(
        restored_pressure.data().getPyArray(), [101325.0, 100980.0]
    )


def test_tutorial_cgns_hdf5_round_trip():
    import noder.core.io as noder_io

    if not hasattr(noder_io, "write_numpy"):
        import pytest

        pytest.skip("Noder was built without HDF5 support")

    # docs:start cgns-round-trip
    from pathlib import Path
    from tempfile import TemporaryDirectory

    x, y = np.meshgrid(np.arange(3.0), np.arange(2.0), indexing="ij")
    zone = new_zone_from_arrays(
        "Wing",
        ["x", "y", "Pressure"],
        [x, y, x + y],
    )
    tree = new_tree(AeroBase=[zone])
    new_node(
        "CGNSLibraryVersion",
        "CGNSLibraryVersion_t",
        np.array([4.0], dtype=np.float32),
        parent=tree,
    )

    with TemporaryDirectory() as temporary_directory:
        filename = Path(temporary_directory) / "wing.cgns"
        tree.write(str(filename))
        restored = noder.read(str(filename))

    restored_pressure = restored.pick().by_name("Pressure")
    # docs:end cgns-round-trip

    assert restored_pressure is not None
    np.testing.assert_array_equal(
        restored_pressure.data().getPyArray(), [[0.0, 1.0], [1.0, 2.0], [2.0, 3.0]]
    )


@pytest.mark.doc_example(target="noder.core.nodeToPyCGNS")
@pytest.mark.doc_example(target="noder.core.pyCGNSToNode")
def test_tutorial_cgns_python_interoperability():
    # docs:start cgns-python
    import numpy as np
    import noder
    from noder import new_tree, new_zone_from_arrays

    x, y = np.meshgrid(np.arange(3.0), np.arange(2.0), indexing="ij")
    zone = new_zone_from_arrays(
        "Wing",
        ["x", "y", "Pressure"],
        [x, y, x + y],
    )
    tree = new_tree(MainBase=[zone])

    # Convert to the conventional [name, value, children, type] CGNS tree form.
    cgns_tree = noder.nodeToPyCGNS(tree)
    restored = noder.pyCGNSToNode(cgns_tree)
    # docs:end cgns-python

    assert cgns_tree[0] == "CGNSTree"
    restored_pressure = restored.pick().by_name("Pressure")
    assert restored_pressure is not None
    np.testing.assert_array_equal(
        restored_pressure.data().getPyArray(), [[0.0, 1.0], [1.0, 2.0], [2.0, 3.0]]
    )
