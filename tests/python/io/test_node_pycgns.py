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
    b.setData(Array(np.array([0])))
    b.attachTo(a)
    c = Node('c', 'd_t')
    c.setData(Array(np.array([1])))
    c.attachTo(a)
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

    a.addChild(b)
    b.addChild(c)
    c.addChild(d)
    d.addChild(e)

    f.addChild(g)
    g.addChild(h)
    f.addChild(i)

    f.attachTo(b)

    a_pycgns = nodeToPyCGNS(a)
    is_empty_pycgns(a_pycgns)
    assert a_pycgns[0] == "a"
    assert a_pycgns[3] == "DataArray_t"
