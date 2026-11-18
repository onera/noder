import numpy as np
import copy
import pytest
from noder.core import Node, Array, nodeToPyCGNS, pyCGNSToNode

def is_empty_pycgns( obj ):
    assert isinstance(obj, list)
    assert len(obj) == 4
    assert isinstance(obj[0],str)
    assert obj[1] is None
    assert isinstance(obj[3],str)
    assert isinstance(obj[2],list)
    for i in obj[2]: is_empty_pycgns(i)

def get_all_paths_list(node, pathlist):
    pathlist += [ node.path() ]
    for child in node.children():
        get_all_paths_list(child, pathlist)

def get_all_paths(node):
    paths = []
    get_all_paths_list(node, paths)
    return paths


def test_pyCGNSToNode_single():
    a = ['a', np.array([1]), [], 'type_t']
    n = pyCGNSToNode(a)


@pytest.mark.parametrize("kind", ["bool", "i1", "i2", "i4", "i8", "u1", "u2", "u4", "u8", "f4", "f8", "c8", "c16"])
def test_pyCGNSToNode_equivalent_dtype_descriptor(kind):
    original = np.dtype(kind)
    descriptor = copy.copy(original)
    assert descriptor is not original and descriptor == original
    values = np.array([0, 1], dtype=descriptor)
    node = pyCGNSToNode(["Data", values, [], "DataArray_t"], strict_types=False)
    restored = nodeToPyCGNS(node, strict_types=False)[1]
    np.testing.assert_array_equal(restored, values)
    assert restored.dtype == original


@pytest.mark.parametrize(
    ("kind", "expected"),
    [("bool", np.int32), ("i1", np.int32), ("i2", np.int32),
     ("u1", np.uint32), ("u2", np.uint32)],
)
def test_pyCGNSToNode_strict_types_promotes_narrow_values(kind, expected):
    values = np.array([0, 1], dtype=kind)
    node = pyCGNSToNode(["Data", values, [], "DataArray_t"])
    assert node.numpy().dtype == np.dtype(expected)
    assert nodeToPyCGNS(node)[1].dtype == np.dtype(expected)


@pytest.mark.parametrize("store_as_s1", [False, True])
def test_nodeToPyCGNS_normalizes_string_payloads_to_c1_arrays(store_as_s1):
    node = Node("ZoneType", "ZoneType_t")
    node.set_data("Structured", store_strings_as_S1=store_as_s1)

    pycgns_node = nodeToPyCGNS(node)
    payload = pycgns_node[1]
    assert payload.dtype == np.dtype("S1")
    assert payload.shape == (len("Structured"),)
    assert b"".join(payload.tolist()) == b"Structured"

    restored = pyCGNSToNode(pycgns_node)
    assert restored.data().extractString() == "Structured"

def test_pyCGNSToNode_tree():
    e = ["e", np.array([5]), [], "d_t"]
    d = ["d", np.array([4]), [], "d_t"]
    c = ["c", np.array([3]), [d,e], "d_t"]
    b = ["b", np.array([2]), [], "d_t"]
    pycgns_tree = ["a", np.array([1]), [b,c], "d_t"]
    t = pyCGNSToNode(pycgns_tree)

    assert get_all_paths(t) == ["a","a/b","a/c","a/c/d","a/c/e"]


def test_toPyCGNS_NodeWithChildren():
    a = Node('a', 'd_t')
    b = Node('b', 'd_t')
    b.set_data(Array(np.array([0])))
    b.attach_to(a)
    c = Node('c', 'd_t')
    c.set_data(Array(np.array([1])))
    c.attach_to(a)
    node_pycgns = nodeToPyCGNS(a)
    assert node_pycgns == ['a', None, [['b', np.array([0]), [], 'd_t'], \
                                       ['c', np.array([1]), [], 'd_t']], 'd_t']


def test_children_pycgns():
    a_pycgns = ['a', None, [['b', None, [], 'd_t'], \
                            ['c', None, [], 'd_t']], 'd_t']
    a_node = pyCGNSToNode( a_pycgns )
    print(len(a_node.children()),len(a_node.children()))
    assert len(a_node.children()) == len(a_node.children())

def test_nodeToPyCGNS_empty():
    a = Node("a", "Type_t")
    a_pycgns = nodeToPyCGNS(a)
    is_empty_pycgns(a_pycgns)

    assert a_pycgns[0] == "a"
    assert a_pycgns[3] == "Type_t"

def test_nodeToPyCGNS_tree():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")
    e = Node("e")
    f = Node("f")
    g = Node("g")
    h = Node("h")
    i = Node("i")

    a.add_child(b)
    b.add_child(c)
    c.add_child(d)
    d.add_child(e)

    f.add_child(g)
    g.add_child(h)
    f.add_child(i)

    f.attach_to(b)

    a_pycgns = nodeToPyCGNS(a)
    is_empty_pycgns(a_pycgns)
    assert a_pycgns[0] == "a"
    assert a_pycgns[3] == "DataArray_t"

def test_link_roundtrip_pycgns():
    link_pycgns = [
        "link",
        ["target_file:other.cgns", "target_path:/CGNSTree/Base"],
        [],
        "DataArray_t",
    ]
    node = pyCGNSToNode(link_pycgns)
    assert node.has_link_target()
    assert node.link_target_file() == "other.cgns"
    assert node.link_target_path() == "/CGNSTree/Base"

    back = nodeToPyCGNS(node)
    assert back[0] == "link"
    assert back[1] == ["target_file:other.cgns", "target_path:/CGNSTree/Base"]

def test_interoperability_retrieval():

    pycgns_node = [
        "root",
        np.array([1,2,3],order='F'),
        [
            ["a",
             None,
             [],
             "DataArray_t"],

             ["b",
             None,
             [],
             "DataArray_t"],
        ],
        "root_t"
    ]

    noder_view = pyCGNSToNode(pycgns_node)
    a_node = noder_view.pick().by_name("a")
    assert a_node.name() == "a"

    root_pycgns_node = nodeToPyCGNS(noder_view)
    assert root_pycgns_node is not pycgns_node
    assert root_pycgns_node[2] is not pycgns_node[2]
    assert root_pycgns_node[1] is pycgns_node[1]

    a_pycgns_node = nodeToPyCGNS(a_node)
    assert a_pycgns_node is not pycgns_node[2][0]


def test_interoperability_ports_copy_structure_and_share_numpy_arrays():
    pycgns_node = [
        "root",
        np.array([1, 2, 3], dtype=np.int32),
        [["child", None, [], "DataArray_t"]],
        "root_t",
    ]

    node = pyCGNSToNode(pycgns_node)
    snapshot = nodeToPyCGNS(node)
    assert snapshot is not pycgns_node
    assert snapshot[2] is not pycgns_node[2]
    assert snapshot[2][0] is not pycgns_node[2][0]
    assert snapshot[1] is pycgns_node[1]

    pycgns_node[0] = "external edit"
    pycgns_node[2].append(["external", None, [], "DataArray_t"])
    assert node.name() == "root"
    assert len(node.children()) == 1

    child = node.pick().by_name("child")
    child.set_name("renamed")
    child.set_data(np.array([4, 5], dtype=np.int32))

    updated_snapshot = nodeToPyCGNS(node)
    assert updated_snapshot is not snapshot
    assert updated_snapshot[0] == "root"
    assert updated_snapshot[2][0][0] == "renamed"
    np.testing.assert_array_equal(updated_snapshot[2][0][1], [4, 5])
    assert pycgns_node[2][0][0] == "child"


def test_empty_numeric_arrays_survive_pycgns_roundtrip():
    values = np.empty((0, 3), dtype=np.int32)
    pycgns_node = ["ElementConnectivity", values, [], "DataArray_t"]

    node = pyCGNSToNode(pycgns_node)
    restored = nodeToPyCGNS(node)

    assert pycgns_node[1] is values
    assert restored is not pycgns_node
    assert restored[1] is values
    assert restored[1].shape == (0, 3)
    assert restored[1].dtype == np.dtype(np.int32)

def test_interoperability_add_child_using_pycgns():

    pycgns_node = [
        "root",
        np.array([1,2,3],order='F'),
        [
            ["a",
             None,
             [],
             "DataArray_t"],

             ["b",
             None,
             [],
             "DataArray_t"],
        ],
        "root_t"
    ]

    noder_view = pyCGNSToNode(pycgns_node)
    a_node = noder_view.pick().by_name("a")
    assert a_node.name() == "a"
    assert nodeToPyCGNS(a_node) is not pycgns_node[2][0]

    pycgns_node[2] += [ ["c", None, [], "DataArray_t"] ]

    noder_view = pyCGNSToNode(pycgns_node)
    c_node = noder_view.pick().by_name("c")
    assert c_node.name() == "c"
    c_pycgns_node = nodeToPyCGNS(c_node)
    assert c_pycgns_node is not pycgns_node[2][2]

def test_interoperability_add_child_using_noder():

    pycgns_node = [
        "root",
        np.array([1,2,3],order='F'),
        [
            ["a",
             None,
             [],
             "DataArray_t"],

             ["b",
             None,
             [],
             "DataArray_t"],
        ],
        "root_t"
    ]

    noder_view = pyCGNSToNode(pycgns_node)
    a_node = noder_view.pick().by_name("a")
    assert a_node.name() == "a"
    root = a_node.parent()
    c = Node("c","DataArray_t")
    root.add_child(c)

    root_snapshot = nodeToPyCGNS(root)
    assert root_snapshot is not pycgns_node
    assert root_snapshot[2][-1] == ["c", None, [], "DataArray_t"]
    assert len(pycgns_node[2]) == 2


def test_interoperability_add_child_using_node_group():

    pycgns_node = [
        "root",
        np.array([1,2,3],order='F'),
        [
            ["a",
             None,
             [],
             "DataArray_t"],

             ["b",
             None,
             [],
             "DataArray_t"],
        ],
        "root_t"
    ]

    noder_view = pyCGNSToNode(pycgns_node)
    a_node = noder_view.pick().by_name("a")
    assert a_node.name() == "a"
    root = a_node.parent()
    c = Node("c","DataArray_t")
    root/c

    root_snapshot = nodeToPyCGNS(root)
    assert root_snapshot is not pycgns_node
    assert root_snapshot[2][-1] == ["c", None, [], "DataArray_t"]
    assert len(pycgns_node[2]) == 2

def test_interoperability_robustness_forbidden_same_name():
    pycgns_node = [
        "root",
        np.array([1,2,3],order='F'),
        [
            ["a",
             None,
             [],
             "DataArray_t"],

             ["a",
             None,
             [],
             "DataArray_t"],

             ["a",
             None,
             [],
             "DataArray_t"],

             ["b",
             None,
             [],
             "DataArray_t"],


        ],
        "root_t"
    ]

    noder_view = pyCGNSToNode(pycgns_node)
    a_node = noder_view.pick().by_name("a")
    assert a_node.name() == "a"

    root_pycgns_node = nodeToPyCGNS(noder_view)
    assert root_pycgns_node is not pycgns_node
    assert len(pycgns_node[2]) == 4
    assert len(root_pycgns_node[2]) == len(noder_view.children())
    assert len({child[0] for child in root_pycgns_node[2]}) == len(root_pycgns_node[2])

    a_pycgns_node = nodeToPyCGNS(a_node)
    assert a_pycgns_node is not pycgns_node[2][0]

    b_pycgns_node = pycgns_node[2][3]
    assert b_pycgns_node[0] == "b"
    assert root_pycgns_node[2][1][0] == "b"

# def test_matrix(tmp_path):
