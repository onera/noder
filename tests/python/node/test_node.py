import numpy as np
from noder.core import Node, Array, pyCGNSToNode, nodeToPyCGNS

def get_all_paths_list(node, pathlist):
    pathlist += [ node.path() ]
    for child in node.children():
        get_all_paths_list(child, pathlist)

def get_all_paths(node):
    paths = []
    get_all_paths_list(node, paths)
    return paths


def test_init():
    a = Node("a")
    b = Node("b", "type_t")

def test_name():
    a = Node("a")
    name = a.name()
    assert name == "a"

def test_set_name():
    a = Node("a")
    a.set_name("b")
    name = a.name()
    assert name == "b"

def test_type():
    a = Node("a", "type_t")
    node_type = a.type()
    assert node_type == "type_t"

def test_set_type():
    a = Node("a")
    a.set_type("NewType_t")
    node_type = a.type()
    assert node_type == "NewType_t"

def test_children_empty():
    a = Node("a")
    children = a.children()
    if len(children) != 0: raise ValueError("failed Python empty children children")

def test_parent_empty():
    a = Node("a")
    parent = a.parent()
    assert parent is None

def test_dataInt():
    a = Node("a")
    arr = Array(np.array([1]))
    a.set_data(arr)
    print(a.data())


def test_root_itself():
    a = Node("a")
    b = a.root()
    assert a is b

def test_level_0():
    a = Node("a")
    level = a.level()
    assert level == 0

def test_position_null():
    a = Node("a")
    pos = a.position()
    assert pos == 0

def test_getPath_itself():
    a = Node("a")
    path = a.path()
    assert path == "a"

def test_attach_to():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    expected_path_of_b = "a/b"
    assert b.path() == expected_path_of_b 

def test_attach_to_multiple_levels():
    max_levels = 20
    nodes = [Node('0', 'type_t')]
    paths = ['0']
    for i in range(1,max_levels):
        node = Node('%d'%i, 'type_t')
        node.attach_to(nodes[-1])
        nodes += [ node ]
        paths += [ paths[-1]+'/'+node.name() ]

        path = node.path()
        assert path == paths[-1]
        
        children_of_parent = nodes[-2].children()
        assert len(children_of_parent) == 1
        assert node is children_of_parent[0]

        parent = node.parent()
        assert parent is nodes[-2]        

def test_add_child():
    a = Node("a")
    b = Node("b")
    a.add_child(b)
    expected_path_of_b = "a/b"
    assert b.path() == expected_path_of_b 

def test_detach_0():
    a = Node("a")
    a.detach()
    assert a.parent() is None

def test_detach_1():
    a = Node('a')
    b = Node('b')
    b.attach_to(a)
    b.detach()
    assert b.parent() is None

    children_of_a = a.children()
    assert len(children_of_a) == 0
    parent_of_b = b.parent()
    assert parent_of_b is None
    path_of_b = b.path()
    assert path_of_b == 'b'

def test_detach_2():
    nb_of_children = 5
    a = Node('a')
    nodes_to_attach = [ Node('%d'%i) for i in range(nb_of_children) ]
    for n in nodes_to_attach: n.attach_to(a)
    children_of_a = a.children()
    
    index_child_to_detach = 2
    assert index_child_to_detach < nb_of_children
    child_to_detach = children_of_a[index_child_to_detach]
    child_to_detach.detach()
    assert len(a.children()) == nb_of_children - 1

    for i, node in enumerate(a.children()):
        if i < index_child_to_detach:
            assert node.name() == str(i)
        else:
            assert node.name() == str(i+1)

def test_detach_3():
    max_levels = 20
    nodes = [Node('0', 'type_t')]
    for i in range(1,max_levels):
        node = Node('%d'%i, 'type_t')
        node.attach_to(nodes[-1])
        nodes += [ node ]
        children_of_parent = nodes[-2].children()

        assert len(children_of_parent) == 1
        assert node is children_of_parent[0]
        parent = node.parent()
        assert parent is nodes[-2]
        path = node.path()
        assert path == '/'.join([n.name() for n in nodes])
    
    index_child_to_detach = 5
    assert index_child_to_detach < max_levels

    child_to_detach = nodes[index_child_to_detach]
    child_to_detach.detach()
    last_child_path = nodes[-1].path()
    expected_path = '/'.join([str(i) for i in range(index_child_to_detach,max_levels)])
    assert last_child_path == expected_path

def test_delete_multiple_descendants():
    max_levels = 20
    nodes = [Node('0', 'type_t')]
    for i in range(1,max_levels):
        node = Node('%d'%i, 'type_t')
        node.attach_to(nodes[-1])
        nodes += [ node ]

    # WARNING important to detach before deleting. 
    index_node_to_delete = 3
    nodes[index_node_to_delete].detach() 
    del nodes[index_node_to_delete]

    expected_number_of_descendants_from_root = index_node_to_delete
    descendants = nodes[0].descendants()
    assert len(descendants) == expected_number_of_descendants_from_root
    print(nodes[1])

def test_getPath():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    b.attach_to(a)
    c.attach_to(b)
    expected_path_of_c = "a/b/c"
    assert c.path() == expected_path_of_c

def test_root():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")
    e = Node("e")

    a.add_child(b)
    b.add_child(c)
    c.add_child(d)
    d.add_child(e)

    assert e.root() is a

def test_level():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")
    e = Node("e")

    a.add_child(b)
    b.add_child(c)
    c.add_child(d)
    d.add_child(e)

    assert e.level() == 4

def test_print_tree():
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

    print(g)


def test_children():
    a = Node('a')
    b = Node('b')
    b.attach_to(a)
    c = Node('c')
    c.attach_to(a)

    assert len(a.children()) == len(a.children())
    assert a.children() is not a.children() # important
    assert len(a.children()) == 2
    for n1, n2 in zip(a.children(), a.children()):
        assert n1 is n2

def test_children_checkPaths():
    a = Node('a')
    b = Node('b')
    b.attach_to(a)
    c = Node('c')
    c.attach_to(a)

    children = a.children()
    expected_children_paths = ['a/b', 'a/c']
    
    i = -1
    for child in children:
        i += 1
        assert child.path() == expected_children_paths[i]
        

def test_position():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")

    b.attach_to(a)
    c.attach_to(a)
    d.attach_to(a)

    assert a.position() == 0
    assert b.position() == 0
    assert c.position() == 1
    assert d.position() == 2


def test_dangerous_extendChildren():
    a = Node("a")
    b = Node("b")
    c = Node("c")

    children_of_a = a.children()
    assert len(children_of_a) == 0

    # dangerous! since hierarchy relationship (parent) not updated
    children_of_a += [b, c] # equivalent to children_of_a.extend([b,c])
    assert len(children_of_a) == 2

    first_child_of_a = children_of_a[0]
    assert first_child_of_a is b

    second_child_of_a = children_of_a[1]
    assert second_child_of_a is c

    # note that hierarchy relationship is not updated:
    assert b.parent() is None # not a
    assert b.path() == "b" # not "a/b"
    assert c.parent() is None # not a
    assert c.path() == "c" # not "a/c"

    # IN CONCLUSION: avoid manipulating directly the children list.
    # Prefer using dedicated methods add_child or attach_to.
    

if __name__ == '__main__':
    test_print_tree()
