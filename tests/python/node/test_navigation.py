import numpy as np
from noder.core import Node

def test_get_childByName():
    a = Node("a")
    b = Node("b")
    b.attachTo(a)
    c = Node("c")
    c.attachTo(a)
    d = Node("d")
    d.attachTo(c)

    n = a.nav().childByName("c")
    assert n.name() == "c"
    assert n is c

    none = a.nav().childByName("d")
    assert none is None


def test_get_byNamePattern():
    a = Node("a")
    b = Node("b")
    b.attachTo(a)
    c = Node("c")
    c.attachTo(a)
    d = Node("abcd")
    d.attachTo(c)

    n = a.nav().byNamePattern("ab\\B")
    assert n.name() == "abcd"
    assert n is d

    none = a.nav().byName(r"(\Bab)")
    assert none is None


def test_get_childByType():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","d_t")
    d.attachTo(c)

    n = a.nav().childByType("c_t")
    assert n.type() == "c_t"
    assert n is c

    none = a.nav().childByType("d_t")
    assert none is None


def test_get_byType():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","d_t")
    d.attachTo(c)

    n = a.nav().byType("d_t")
    assert n.type() == "d_t"
    assert n is d

    none = a.nav().byType("e_t")
    assert none is None


def test_get_byTypePattern():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attachTo(a)
    c = Node("c","c_t")
    c.attachTo(a)
    d = Node("d","abcd_t")
    d.attachTo(c)

    n = a.nav().byTypePattern("ab\\B")
    assert n.type() == "abcd_t"
    assert n is d

    none = a.nav().byName(r"(\Bab)")
    assert none is None