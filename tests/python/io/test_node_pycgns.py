import numpy as np
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
    assert root_pycgns_node is pycgns_node

    a_pycgns_node = nodeToPyCGNS(a_node)
    assert a_pycgns_node is pycgns_node[2][0]

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
    a_pycgns_node = nodeToPyCGNS(a_node)
    assert a_pycgns_node is pycgns_node[2][0]

    pycgns_node[2] += [ ["c", None, [], "DataArray_t"] ]

    noder_view = pyCGNSToNode(pycgns_node)
    c_node = noder_view.pick().by_name("c")
    assert c_node.name() == "c"
    c_pycgns_node = nodeToPyCGNS(c_node)
    assert c_pycgns_node is pycgns_node[2][2]

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

    assert nodeToPyCGNS(root) is pycgns_node
    assert nodeToPyCGNS(c) is pycgns_node[2][2]
    assert pycgns_node[2][2] == ["c", None, [], "DataArray_t"]


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

    assert nodeToPyCGNS(root) is pycgns_node
    assert nodeToPyCGNS(c) is pycgns_node[2][2]
    assert pycgns_node[2][2] == ["c", None, [], "DataArray_t"]

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
    assert root_pycgns_node is pycgns_node

    a_pycgns_node = nodeToPyCGNS(a_node)
    assert a_pycgns_node is pycgns_node[2][0]

    b_pycgns_node = pycgns_node[2][1]
    assert b_pycgns_node[0] == "b"
