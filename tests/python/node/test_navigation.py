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

def test_by_and():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c","TypeT")
    c.set_data("value")
    c.attach_to(b)
    d = Node("d")
    d.attach_to(c)

    n = a.pick().by_and("c","TypeT","value")
    assert n.name() == "c"
    assert n is c

def test_all_by_and():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c", "TypeT")
    c.set_data("value")
    c.attach_to(b)
    d = Node("d", "TypeT")
    d.set_data("value")
    d.attach_to(c)

    matches = a.pick().all_by_and(type="TypeT", data="value")
    assert len(matches) == 2
    assert matches[0] is c
    assert matches[1] is d

    none = a.pick().all_by_and(type="TypeT", data="missing")
    assert len(none) == 0

def test_all_by_and_dispatcher_scalar():
    a = Node("a")
    b = Node("b")
    b.attach_to(a)
    c = Node("c", "TypeT")
    c.set_data(3.5)
    c.attach_to(b)
    d = Node("d", "TypeT")
    d.set_data(3.5)
    d.attach_to(c)

    matches = a.pick().all_by_and("", "TypeT", np.float32(3.5))
    assert len(matches) == 2
    assert matches[0] is c
    assert matches[1] is d

    with pytest.raises(TypeError):
        a.pick().all_by_and("", "TypeT", {"unexpected": "type"})


def _build_navigation_example_tree():
    root = Node("root", "CGNSTree_t")

    zone_a = Node("Zone_A", "Zone_t")
    zone_a.set_data("prefix_A")
    zone_b = Node("Zone_B", "Zone_t")
    zone_b.set_data("prefix_B")
    root.add_children([zone_a, zone_b])

    coords_a = Node("Coords", "DataArray_t")
    coords_a.set_data(3)
    marker = Node("Marker", "UserDefinedData_t")
    marker.set_data(7)
    zone_a.add_children([coords_a, marker])

    coords_b = Node("Coords", "DataArray_t")
    coords_b.set_data(3)
    zone_b.add_child(coords_b)

    return root


def test_child_by_name_example():
    # docs:start child_by_name_example
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_name("Zone_A")
    assert node is not None
    assert node.name() == "Zone_A"
    # docs:end child_by_name_example


def test_by_name_example():
    # docs:start by_name_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name("Marker")
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_name_example


def test_all_by_name_example():
    # docs:start all_by_name_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name("Coords")
    assert len(matches) == 2
    # docs:end all_by_name_example


def test_by_name_regex_example():
    # docs:start by_name_regex_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name_regex(r"Zone_\w")
    assert node is not None
    assert node.name().startswith("Zone_")
    # docs:end by_name_regex_example


def test_all_by_name_regex_example():
    # docs:start all_by_name_regex_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name_regex(r"Zone_\w")
    assert len(matches) == 2
    # docs:end all_by_name_regex_example


def test_by_name_glob_example():
    # docs:start by_name_glob_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_name_glob("Zone_*")
    assert node is not None
    assert node.name().startswith("Zone_")
    # docs:end by_name_glob_example


def test_all_by_name_glob_example():
    # docs:start all_by_name_glob_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_name_glob("Zone_*")
    assert len(matches) == 2
    # docs:end all_by_name_glob_example


def test_child_by_type_example():
    # docs:start child_by_type_example
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_type("Zone_t")
    assert node is not None
    assert node.type() == "Zone_t"
    # docs:end child_by_type_example


def test_by_type_example():
    # docs:start by_type_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type("UserDefinedData_t")
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_type_example


def test_all_by_type_example():
    # docs:start all_by_type_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type("DataArray_t")
    assert len(matches) == 2
    # docs:end all_by_type_example


def test_by_type_regex_example():
    # docs:start by_type_regex_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type_regex(r".*Defined.*")
    assert node is not None
    assert node.type() == "UserDefinedData_t"
    # docs:end by_type_regex_example


def test_all_by_type_regex_example():
    # docs:start all_by_type_regex_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type_regex(r".*Array.*")
    assert len(matches) == 2
    # docs:end all_by_type_regex_example


def test_by_type_glob_example():
    # docs:start by_type_glob_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_type_glob("*Defined*")
    assert node is not None
    assert node.type() == "UserDefinedData_t"
    # docs:end by_type_glob_example


def test_all_by_type_glob_example():
    # docs:start all_by_type_glob_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_type_glob("*Array*")
    assert len(matches) == 2
    # docs:end all_by_type_glob_example


def test_child_by_data_example():
    # docs:start child_by_data_example
    tree = _build_navigation_example_tree()
    node = tree.pick().child_by_data("prefix_B")
    assert node is not None
    assert node.name() == "Zone_B"
    # docs:end child_by_data_example


def test_by_data_example():
    # docs:start by_data_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_data("prefix_A")
    assert node is not None
    assert node.name() == "Zone_A"
    # docs:end by_data_example


def test_by_data_glob_example():
    # docs:start by_data_glob_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_data_glob("prefix_B")
    assert node is not None
    assert node.name() == "Zone_B"
    # docs:end by_data_glob_example


def test_all_by_data_example():
    # docs:start all_by_data_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_data(3)
    assert len(matches) == 2
    # docs:end all_by_data_example


def test_all_by_data_glob_example():
    # docs:start all_by_data_glob_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_data_glob("prefix_*")
    assert len(matches) == 2
    # docs:end all_by_data_glob_example


def test_by_and_example():
    # docs:start by_and_example
    tree = _build_navigation_example_tree()
    node = tree.pick().by_and(name="Marker", type="UserDefinedData_t", data=7)
    assert node is not None
    assert node.name() == "Marker"
    # docs:end by_and_example


def test_all_by_and_example():
    # docs:start all_by_and_example
    tree = _build_navigation_example_tree()
    matches = tree.pick().all_by_and(type="DataArray_t", data=3)
    assert len(matches) == 2
    # docs:end all_by_and_example
