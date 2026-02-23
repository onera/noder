import noder.tests.node_group as test_in_cpp


def test_cpp_operator_plus_attachToFirstParent():
    return test_in_cpp.test_operator_plus_attachToFirstParent()


def test_cpp_operator_div_addChild_chain():
    return test_in_cpp.test_operator_div_addChild_chain()


def test_cpp_operator_div_with_plus_group():
    return test_in_cpp.test_operator_div_with_plus_group()


def test_cpp_operator_nested_expression_tree():
    return test_in_cpp.test_operator_nested_expression_tree()
