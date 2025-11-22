import numpy as np
from noder.core import Node

def test_get_child_by_name():
    a = Node("a")
    b = Node("b")
    b.attachTo(a)
    c = Node("c")
    c.attachTo(a)
    d = Node("d")
    d.attachTo(c)

    n = a.pick().child_by_name("c")
    assert n.name() == "c"
    assert n is c

    none = a.pick().child_by_name("d")
    assert none is None


def test_get_by_name_pattern():
    a = Node("a")
    b = Node("b")
    b.attachTo(a)
    c = Node("c")
    c.attachTo(a)
    d = Node("abcd")
    d.attachTo(c)

    n = a.pick().by_name_pattern("ab\\B")
    assert n.name() == "abcd"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None


def test_get_child_by_type():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","d_t")
    d.attachTo(c)

    n = a.pick().child_by_type("c_t")
    assert n.type() == "c_t"
    assert n is c

    none = a.pick().child_by_type("d_t")
    assert none is None


def test_get_by_type():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","d_t")
    d.attachTo(c)

    n = a.pick().by_type("d_t")
    assert n.type() == "d_t"
    assert n is d

    none = a.pick().by_type("e_t")
    assert none is None


def test_get_by_type_pattern():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","abcd_t")
    d.attachTo(c)

    n = a.pick().by_type_pattern("ab\\B")
    assert n.type() == "abcd_t"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None