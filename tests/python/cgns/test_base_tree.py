import numpy as np
import pytest

from noder.core import Node, nodeToPyCGNS, pyCGNSToNode
from noder.cgns import Base, Tree, Zone, add, merge, new_base, new_zone_from_arrays, new_zone_from_dict


def make_cart(name="block", shape=(3, 3, 3)):
    values = np.ones(shape, dtype=np.float64)

    zone = Zone(name)
    coordinates = Node("GridCoordinates", "GridCoordinates_t")

    for coordinate in ["CoordinateX", "CoordinateY", "CoordinateZ"]:
        child = Node(coordinate, "DataArray_t")
        child.set_data(values.copy())
        coordinates.add_child(child)

    zone.add_child(coordinates)
    zone.update_shape()
    return zone


def test_base_init_and_new_base():
    zone_a = make_cart("ZoneA")
    zone_b = make_cart("ZoneB", (4, 3, 2))

    base = new_base("Base", [zone_a, zone_b])
    assert isinstance(base, Base)
    assert base.name() == "Base"
    assert base.type() == "CGNSBase_t"
    assert base.dim() == 3
    assert base.physical_dimension() == 3
    assert base.number_of_zones() == 2
    assert [z.name() for z in base.zones()] == ["ZoneA", "ZoneB"]


def test_cgns_helpers_are_exported_from_package_root():
    from noder import add as root_add
    from noder import Base as RootBase
    from noder import merge as root_merge
    from noder import Tree as RootTree
    from noder import Zone as RootZone
    from noder import new_base as root_new_base
    from noder import new_zone_from_arrays as root_new_zone_from_arrays
    from noder import new_zone_from_dict as root_new_zone_from_dict

    assert RootBase is Base
    assert RootTree is Tree
    assert RootZone is Zone
    assert root_new_base is new_base
    assert root_new_zone_from_arrays is new_zone_from_arrays
    assert root_new_zone_from_dict is new_zone_from_dict
    assert root_add is add
    assert root_merge is merge


def test_new_zone_from_arrays_and_dict():
    x, y = np.meshgrid(np.arange(3.0), np.arange(2.0), indexing="ij")
    z = np.zeros_like(x)
    pressure = x + y

    zone = new_zone_from_arrays("Block", ["x", "y", "z", "Pressure"], [x, y, z, pressure])
    assert isinstance(zone, Zone)
    assert zone.name() == "Block"
    assert zone.dim() == 2
    assert zone.number_of_points() == 6
    assert zone.number_of_cells() == 2
    assert zone.x().shape == (3, 2)
    assert np.array_equal(zone.field("Pressure", behavior_if_not_found="raise"), pressure)

    zone_from_dict = new_zone_from_dict("DictBlock", {"x": x, "y": y, "z": z, "Pressure": pressure})
    assert isinstance(zone_from_dict, Zone)
    assert zone_from_dict.name() == "DictBlock"
    assert np.array_equal(zone_from_dict.field("Pressure", behavior_if_not_found="raise"), pressure)


def test_add_and_merge_helpers():
    zone_a = make_cart("A")
    zone_b = make_cart("B")
    zone_c = make_cart("C")

    tree = add(zone_a, SecondBase=[zone_b, zone_c])
    assert isinstance(tree, Tree)
    assert tree.number_of_bases() == 2
    assert tree.base_names() == ["Base", "SecondBase"]
    assert [zone.name() for zone in tree.base("Base").zones()] == ["A"]
    assert [zone.name() for zone in tree.base("SecondBase").zones()] == ["B", "C"]

    left = Node("Root")
    left.add_child(Node("left"))
    right = Node("Root")
    right.add_child(Node("right"))

    merged = merge([left, right])
    assert merged is not left
    assert merged.name() == "Root"
    assert set(merged.get_children_names()) == {"left", "right"}


def test_new_base_rejects_incoherent_zone_dimensions():
    with pytest.raises(ValueError):
        new_base("Base", [make_cart("Zone3D"), make_cart("Zone2D", (3, 3))])


def test_base_aggregates_fields_and_counts():
    base = new_base("Base", [make_cart("ZoneA"), make_cart("ZoneB", (4, 3, 2))])

    assert base.is_structured()
    assert base.number_of_points() == 27 + 24
    assert base.number_of_cells() == 8 + 6

    fields = base.new_fields("q")
    assert set(fields) == {"ZoneA", "ZoneB"}
    assert fields["ZoneA"]["q"].shape == (3, 3, 3)
    assert fields["ZoneB"]["q"].shape == (4, 3, 2)

    all_fields = base.all_fields(include_coordinates=False)
    assert set(all_fields) == {"ZoneA", "ZoneB"}
    assert set(all_fields["ZoneA"]) == {"q"}


def test_tree_aggregates_bases_and_zones():
    base = new_base("Base", [make_cart("ZoneA")])
    zone_b = make_cart("ZoneB", (4, 3, 2))

    tree = Tree()
    tree.add(base)
    tree.add(zone_b)

    assert tree.name() == "CGNSTree"
    assert tree.type() == "CGNSTree_t"
    assert tree.number_of_bases() == 1
    assert tree.number_of_zones() == 2
    assert tree.zone_names() == ["ZoneA", "ZoneB"]
    assert tree.number_of_points() == 27 + 24

    fields = tree.new_fields(["q", "r"])
    assert set(fields) == {"ZoneA", "ZoneB"}
    assert set(fields["ZoneA"]) == {"q", "r"}


def test_pycgns_converter_returns_specialized_base_and_tree():
    tree = Tree()
    tree.add(new_base("Base", [make_cart("ZoneA")]))

    pycgns_tree = nodeToPyCGNS(tree)
    tree_again = pyCGNSToNode(pycgns_tree)

    assert isinstance(tree_again, Tree)
    assert isinstance(tree_again.children()[0], Base)
    assert isinstance(tree_again.children()[0].children()[0], Zone)


def test_pycgns_converter_with_custom_tree():

    pycgns_list = [
        'CGNSTree',
        None,
        [

            ['Base1',
            np.array([3,3],dtype=np.int32,order='F'),
            [
                ['ZoneA',
                np.array([[16,9,0]],dtype=np.int32,order='F'),
                [
                    ['GridCoordinates',
                    None,
                    [
                        ['CoordinateX',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateY',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateZ',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                    ],
                    'GridCoordinates_t'
                    ]
                ],
                'Zone_t'],
                ['ZoneB',
                np.array([[16,9,0]],dtype=np.int32,order='F'),
                [
                    ['GridCoordinates',
                    None,
                    [
                        ['CoordinateX',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateY',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateZ',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                    ],
                    'GridCoordinates_t'
                    ]
                ],
                'Zone_t'],
            ],
            'CGNSBase_t'],

            ['Base2',
            np.array([3,3],dtype=np.int32,order='F'),
            [
                ['ZoneD',
                np.array([[16,9,0]],dtype=np.int32,order='F'),
                [
                    ['GridCoordinates',
                    None,
                    [
                        ['CoordinateX',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateY',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateZ',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                    ],
                    'GridCoordinates_t'
                    ]
                ],
                'Zone_t'],
                ['ZoneE',
                np.array([[16,9,0]],dtype=np.int32,order='F'),
                [
                    ['GridCoordinates',
                    None,
                    [
                        ['CoordinateX',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateY',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                        ['CoordinateZ',
                            np.zeros((4,4),dtype=float, order='F'),
                            [],
                            'DataArray_t'
                        ],
                    ],
                    'GridCoordinates_t'
                    ]
                ],
                'Zone_t'],
            ],
            'CGNSBase_t'],


        ],
        'CGNSTree_t'
    ]


    t = pyCGNSToNode(pycgns_list)
    reached = False
    assert isinstance(t,Tree)
    for b in t.bases():
        assert isinstance(b,Base)
        for z in b.zones():
            assert isinstance(z,Zone)
            for n in z.pick().all_by_and(type='DataArray_t'):
                assert isinstance(n, Node)
                reached = True
    assert reached
