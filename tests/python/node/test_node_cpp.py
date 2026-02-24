import pytest
import os

import noder.tests.node as test_in_cpp
from noder.core import Node

def test_cpp_init(): return test_in_cpp.test_init()

def test_cpp_name(): return test_in_cpp.test_name()

def test_cpp_setName(): return test_in_cpp.test_setName()

def test_cpp_type(): return test_in_cpp.test_type()

def test_cpp_setType(): return test_in_cpp.test_setType()

def test_binding_setNameAndTypeFromPython():
    a = Node("a")
    test_in_cpp.test_binding_setNameAndTypeFromPython(a)
    assert a.name() == "NewName"
    assert a.type() == "NewType"

def test_noData(): test_in_cpp.test_noData()

def test_cpp_children_empty(): return test_in_cpp.test_children_empty()

def test_cpp_parent_empty(): return test_in_cpp.test_parent_empty()

def test_cpp_root_itself(): return test_in_cpp.test_root_itself()

def test_cpp_level_0(): return test_in_cpp.test_level_0()

def test_cpp_position_null(): return test_in_cpp.test_position_null()

def test_cpp_getPath_itself(): return test_in_cpp.test_getPath_itself()

def test_cpp_attachTo(): return test_in_cpp.test_attachTo()

def test_cpp_attachTo_multiple_levels(): test_in_cpp.test_attachTo_multiple_levels()

def test_cpp_addChild(): return test_in_cpp.test_addChild()

def test_cpp_detach_0(): return test_in_cpp.test_detach_0() 

def test_cpp_detach_1(): return test_in_cpp.test_detach_1()

def test_cpp_detach_2(): return test_in_cpp.test_detach_2()

def test_cpp_detach_3(): return test_in_cpp.test_detach_3()

def test_cpp_delete_multiple_descendants(): return test_in_cpp.test_delete_multiple_descendants()

def test_cpp_getPath(): return test_in_cpp.test_getPath()

def test_cpp_root(): return test_in_cpp.test_root()

def test_cpp_level(): return test_in_cpp.test_level()

@pytest.mark.skipif(os.getenv("ENABLE_CPP_PRINT_TEST") != "1", reason="test_cpp_printTree disabled by default, to enable set ENABLE_CPP_PRINT_TEST=1")
def test_cpp_printTree(capsys):
    with capsys.disabled():
        test_in_cpp.test_printTree()

def test_cpp_children(): return test_in_cpp.test_children()

def test_binding_addChildrenFromPython():
    a = Node("a")
    test_in_cpp.test_binding_addChildrenFromPython(a)

    children_of_a = a.children()
    b = children_of_a[0]
    assert b.name() == "b"
    d = children_of_a[1]
    assert d.name() == "d"

    children_of_b = b.children()
    c = children_of_b[0]
    assert c.name() == "c"

    print(c)

def test_cpp_position(): return test_in_cpp.test_position()

def test_link_metadata_api():
    node = Node("link")
    assert not node.has_link_target()

    node.set_link_target("other.cgns", "/CGNSTree/Base")
    assert node.has_link_target()
    assert node.link_target_file() == "other.cgns"
    assert node.link_target_path() == "/CGNSTree/Base"

    node.clear_link_target()
    assert not node.has_link_target()
    assert node.link_target_file() == ""
    assert node.link_target_path() == ""

if __name__ == '__main__':
    test_cpp_printTree()
