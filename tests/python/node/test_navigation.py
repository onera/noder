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


def test_get_by_name_regex():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("abcd")
    d.attach_to(c)

    n = a.pick().by_name_regex("ab\\B")
    assert n.name() == "abcd"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None


def test_get_all_by_name():
    a = Node("a")
    b = Node("target")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("target")
    d.attach_to(c)
    e = Node("target")
    e.attach_to(d)

    matches = a.pick().all_by_name("target")
    assert len(matches) == 3
    assert matches[0] is b
    assert matches[1] is d
    assert matches[2] is e

    none = a.pick().all_by_name("missing")
    assert len(none) == 0


def test_get_all_by_name_regex():
    a = Node("a")
    b = Node("ab_one")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("ab_two")
    d.attach_to(c)

    matches = a.pick().all_by_name_regex("^ab_.*")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_name_regex("^zz_.*")
    assert len(none) == 0


def test_get_by_name_glob():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("abcd")
    d.attach_to(c)
    e = Node("zbcy")
    e.attach_to(c)

    n = a.pick().by_name_glob("ab*")
    assert n.name() == "abcd"
    assert n is d

    n = a.pick().by_name_glob("*bc*")
    assert n.name() == "abcd"
    assert n is d

    none = a.pick().by_name_glob("zz*")
    assert none is None


def test_get_all_by_name_glob():
    a = Node("a")
    b = Node("ab_one")
    b.attach_to(a)
    c = Node("c")
    c.attach_to(a)
    d = Node("ab_two")
    d.attach_to(c)

    matches = a.pick().all_by_name_glob("ab_*")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_name_glob("zz_*")
    assert len(none) == 0


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


def test_get_all_by_type():
    a = Node("a","a_t")
    b = Node("b","target_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","target_t")
    d.attach_to(c)

    matches = a.pick().all_by_type("target_t")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_type("missing_t")
    assert len(none) == 0


def test_get_by_type_regex():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","abcd_t")
    d.attach_to(c)

    n = a.pick().by_type_regex("ab\\B")
    assert n.type() == "abcd_t"
    assert n is d

    none = a.pick().by_name(r"(\Bab)")
    assert none is None


def test_get_all_by_type_regex():
    a = Node("a","a_t")
    b = Node("b","ab_one_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","ab_two_t")
    d.attach_to(c)

    matches = a.pick().all_by_type_regex("^ab_.*_t$")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_type_regex("^zz_.*_t$")
    assert len(none) == 0


def test_get_by_type_glob():
    a = Node("a","a_t")
    b = Node("b","b_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","abcd_t")
    d.attach_to(c)

    n = a.pick().by_type_glob("ab*")
    assert n.type() == "abcd_t"
    assert n is d

    none = a.pick().by_type_glob("zz*")
    assert none is None


def test_get_all_by_type_glob():
    a = Node("a","a_t")
    b = Node("b","ab_one_t")
    b.attach_to(a)
    c = Node("c","c_t")
    c.attach_to(a)
    d = Node("d","ab_two_t")
    d.attach_to(c)

    matches = a.pick().all_by_type_glob("ab_*_t")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_type_glob("zz_*_t")
    assert len(none) == 0


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


def test_get_all_by_data():
    a = Node("a","a_t")
    a.set_data("root")
    b = Node("b","b_t")
    b.set_data("target")
    b.attach_to(a)
    c = Node("c","c_t")
    c.set_data("mid")
    c.attach_to(a)
    d = Node("d","d_t")
    d.set_data("target")
    d.attach_to(c)

    matches = a.pick().all_by_data("target")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_data("missing")
    assert len(none) == 0


def test_get_by_data_glob():
    a = Node("a","a_t")
    a.set_data("a_d")
    b = Node("b","b_t")
    b.set_data("b_d")
    b.attach_to(a)
    c = Node("c","c_t")
    c.set_data("c_d")
    c.attach_to(a)
    d = Node("d","d_t")
    d.set_data("requested_value_42")
    d.attach_to(c)

    n = a.pick().by_data_glob("requested_*_4?")
    assert n.data().extractString() == "requested_value_42"
    assert n is d

    none = a.pick().by_data_glob("missing*")
    assert none is None


def test_get_all_by_data_glob():
    a = Node("a","a_t")
    a.set_data("root")
    b = Node("b","b_t")
    b.set_data("requested_value_01")
    b.attach_to(a)
    c = Node("c","c_t")
    c.set_data("mid")
    c.attach_to(a)
    d = Node("d","d_t")
    d.set_data("requested_value_42")
    d.attach_to(c)

    matches = a.pick().all_by_data_glob("requested_*_??")
    assert len(matches) == 2
    assert matches[0] is b
    assert matches[1] is d

    none = a.pick().all_by_data_glob("missing*")
    assert len(none) == 0


def test_get_by_data_dispatcher_scalar_direct():
    a = Node("a")
    a.set_data(123)

    b = Node("b")
    b.set_data(7)
    b.attach_to(a)

    c = Node("c")
    c.set_data(3.5)
    c.attach_to(a)

    d = Node("d")
    d.set_data(True)
    d.attach_to(c)

    n_int_builtin = a.pick().by_data(7)
    assert n_int_builtin.name() == "b"
    assert n_int_builtin is b

    n_int_numpy = a.pick().by_data(np.int32(7))
    assert n_int_numpy.name() == "b"
    assert n_int_numpy is b

    n_float_builtin = a.pick().by_data(3.5)
    assert n_float_builtin.name() == "c"
    assert n_float_builtin is c

    n_float_numpy = a.pick().by_data(np.float32(3.5))
    assert n_float_numpy.name() == "c"
    assert n_float_numpy is c

    n_bool = a.pick().by_data(np.bool_(True))
    assert n_bool.name() == "d"
    assert n_bool is d

    none = a.pick().by_data(999)
    assert none is None

    with pytest.raises(TypeError):
        a.pick().by_data({"unexpected": "type"})


def test_get_all_by_data_dispatcher_scalar_direct():
    a = Node("a")
    a.set_data(123)

    b = Node("b")
    b.set_data(7)
    b.attach_to(a)

    c = Node("c")
    c.set_data(7)
    c.attach_to(a)

    d = Node("d")
    d.set_data(3.5)
    d.attach_to(c)

    e = Node("e")
    e.set_data(True)
    e.attach_to(c)

    ints = a.pick().all_by_data(7)
    assert len(ints) == 2
    assert ints[0] is b
    assert ints[1] is c

    ints_np = a.pick().all_by_data(np.int32(7))
    assert len(ints_np) == 2
    assert ints_np[0] is b
    assert ints_np[1] is c

    floats = a.pick().all_by_data(3.5)
    assert len(floats) == 1
    assert floats[0] is d

    floats_np = a.pick().all_by_data(np.float32(3.5))
    assert len(floats_np) == 1
    assert floats_np[0] is d

    bools = a.pick().all_by_data(np.bool_(True))
    assert len(bools) == 1
    assert bools[0] is e

    none = a.pick().all_by_data(999)
    assert len(none) == 0

    with pytest.raises(TypeError):
        a.pick().all_by_data({"unexpected": "type"})
