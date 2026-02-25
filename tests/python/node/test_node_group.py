from noder.core import Node


def get_all_paths_list(node, pathlist):
    pathlist += [node.path()]
    for child in node.children():
        get_all_paths_list(child, pathlist)


def get_all_paths(node):
    paths = []
    get_all_paths_list(node, paths)
    return paths


def test_operator_plus_attach_to_first_parent():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")

    b.attach_to(a)
    d.attach_to(c)

    _group = b + c + d

    assert b.parent() is a
    assert c.parent() is a
    assert d.parent() is a
    assert [child.name() for child in a.children()] == ["b", "c", "d"]


def test_operator_div_add_child_chain():
    a = Node("a")
    b = Node("b")
    c = Node("c")

    end = a / b / c
    assert end is c
    assert b.parent() is a
    assert c.parent() is b


def test_operator_div_with_plus_group():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")
    e = Node("e")

    end = a / b / (c + d + e)
    assert end is e
    assert b.parent() is a
    assert [child.name() for child in b.children()] == ["c", "d", "e"]
    assert c.parent() is b
    assert d.parent() is b
    assert e.parent() is b


def test_operator_nested_expression_tree():
    a = Node("a")
    b = Node("b")
    c = Node("c")
    d = Node("d")
    e = Node("e")
    f = Node("f")
    g = Node("g")
    h = Node("h")

    end = a / (b + (c / (e + (f / g) + h)) + d)
    assert end is d

    paths = get_all_paths(a)
    assert paths == [
        "a",
        "a/b",
        "a/c",
        "a/c/e",
        "a/c/f",
        "a/c/f/g",
        "a/c/h",
        "a/d",
    ]


def test_operator_plus_example():
    # docs:start node_group_operator_plus_example
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    c = Node("c")
    _group = b + c
    a / _group
    assert [child.name() for child in a.children()] == ["b", "c"]
    # docs:end node_group_operator_plus_example


def test_operator_div_chain_example():
    # docs:start node_group_operator_div_chain_example
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    c = Node("c")
    end = a / b / c
    assert end is c
    assert c.path() == "a/b/c"
    # docs:end node_group_operator_div_chain_example


def test_operator_div_group_example():
    # docs:start node_group_operator_div_group_example
    from noder.core import Node

    root = Node("root")
    branch = Node("branch")
    leaf_a = Node("leaf_a")
    leaf_b = Node("leaf_b")
    root / branch / (leaf_a + leaf_b)
    assert [child.name() for child in branch.children()] == ["leaf_a", "leaf_b"]
    # docs:end node_group_operator_div_group_example


def test_node_group_nodes_example():
    # docs:start node_group_nodes_example
    from noder.core import Node

    a = Node("a")
    b = Node("b")
    group = a + b
    assert [node.name() for node in group.nodes()] == ["a", "b"]
    # docs:end node_group_nodes_example
