import noder.tests.node_group as test_in_cpp


def test_cpp_operator_plus_attachToFirstParent():
    return test_in_cpp.test_operator_plus_attachToFirstParent()


def test_cpp_operator_div_addChild_chain():
    return test_in_cpp.test_operator_div_addChild_chain()


def test_cpp_operator_div_with_plus_group():
    return test_in_cpp.test_operator_div_with_plus_group()


def test_cpp_operator_nested_expression_tree():
    return test_in_cpp.test_operator_nested_expression_tree()


def test_cpp_nodes_accessor():
    return test_in_cpp.test_nodes_accessor()


def test_cpp_empty_group():
    return test_in_cpp.test_empty_group()
