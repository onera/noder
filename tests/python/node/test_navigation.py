import numpy as np
import pytest
from noder.core import Node, Array

def test_get_child_by_name():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("d")
    d.attach_to(c)

    n = a.pick().child_by_name("c")
    assert n.name() == "c"
    assert n is c

    none = a.pick().child_by_name("d")
    assert none is None


def test_get_by_name_pattern():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("abcd")
    d.attach_to(c)

    n = a.pick().by_name_pattern("ab\\B")
    assert n.name() == "abcd"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None


def test_get_child_by_type():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","d_t")
    d.attach_to(c)

    n = a.pick().child_by_type("c_t")
    assert n.type() == "c_t"
    assert n is c

    none = a.pick().child_by_type("d_t")
    assert none is None


def test_get_by_type():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","d_t")
    d.attach_to(c)

    n = a.pick().by_type("d_t")
    assert n.type() == "d_t"
    assert n is d

    none = a.pick().by_type("e_t")
    assert none is None


def test_get_by_type_pattern():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","abcd_t")
    d.attach_to(c)

    n = a.pick().by_type_pattern("ab\\B")
    assert n.type() == "abcd_t"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None


def test_get_child_by_data():
    a = Node("a")
    b = Node("b")
    b.set_data(Array("test"))
    b.attach_to(a)

    c = Node("c")
    c.set_data(Array(np.ndarray([0],dtype=float)))
    c.attach_to(a)

    d = Node("d")
    d.set_data(Array(np.zeros((3,3),dtype=int)))
    d.attach_to(c)

    e = Node("e")
    e.set_data(Array("requested"))
    e.attach_to(a)


    n = a.pick().child_by_data("requested")
    assert n.name() == "e"
    assert n is e

    none = a.pick().child_by_data("d")
    assert none is None


def test_get_child_by_data_dispatcher_scalar():
    a = Node("a")

    b = Node("b")
    b.set_data(Array(np.array([7], dtype=np.int32)))
    b.attach_to(a)

    c = Node("c")
    c.set_data(Array(np.array([3.5], dtype=np.float32)))
    c.attach_to(a)

    d = Node("d")
    d.set_data(Array(np.array([True], dtype=np.bool_)))
    d.attach_to(a)

    n_int_builtin = a.pick().child_by_data(7)
    assert n_int_builtin.name() == "b"
    assert n_int_builtin is b

    n_int_numpy = a.pick().child_by_data(np.int32(7))
    assert n_int_numpy.name() == "b"
    assert n_int_numpy is b

    n_float_builtin = a.pick().child_by_data(3.5)
    assert n_float_builtin.name() == "c"
    assert n_float_builtin is c

    n_float_numpy = a.pick().child_by_data(np.float32(3.5))
    assert n_float_numpy.name() == "c"
    assert n_float_numpy is c

    n_bool = a.pick().child_by_data(np.bool_(True))
    assert n_bool.name() == "d"
    assert n_bool is d

    with pytest.raises(TypeError):
        a.pick().child_by_data({"unexpected": "type"})


def test_get_child_by_data_dispatcher_scalar_direct():
    a = Node("a")

    b = Node("b")
    b.set_data(7)
    b.attach_to(a)

    c = Node("c")
    c.set_data(3.5)
    c.attach_to(a)

    d = Node("d")
    d.set_data(True)
    d.attach_to(a)

    n_int_builtin = a.pick().child_by_data(7)
    assert n_int_builtin.name() == "b"
    assert n_int_builtin is b

    n_int_numpy = a.pick().child_by_data(7)
    assert n_int_numpy.name() == "b"
    assert n_int_numpy is b

    n_float_builtin = a.pick().child_by_data(3.5)
    assert n_float_builtin.name() == "c"
    assert n_float_builtin is c

    n_float_numpy = a.pick().child_by_data(3.5)
    assert n_float_numpy.name() == "c"
    assert n_float_numpy is c

    n_bool = a.pick().child_by_data(True)
    assert n_bool.name() == "d"
    assert n_bool is d

    with pytest.raises(TypeError):
        a.pick().child_by_data({"unexpected": "type"})


def test_get_by_data():
    a = Node("a","a_t")
    a.set_data("a_d")
    b = Node("b","b_t")
    b.set_data("b_d")
    b.attach_to(a)
    c = Node("c","c_t")
    c.set_data("c_d")
    c.attach_to(a)
    d = Node("d","d_t")
    d.set_data("d_d")
    d.attach_to(c)

    n = a.pick().by_data("d_d")
    assert n.data().extractString() == "d_d"
    assert n is d

    none = a.pick().by_data("e_d")
    assert none is None
