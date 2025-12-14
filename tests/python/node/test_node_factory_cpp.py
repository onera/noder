import pytest
import noder.tests.node_factory as test_in_cpp

def test_cpp_newNodeNoArgs(): return test_in_cpp.test_newNodeNoArgs()
def test_cpp_newNodeOnlyName(): return test_in_cpp.test_newNodeOnlyName()
def test_cpp_newNodeNameAndType(): return test_in_cpp.test_newNodeNameAndType()
def test_cpp_newNodeNameTypeAndData(): return test_in_cpp.test_newNodeNameTypeAndData()

if __name__ == '__main__':
    test_cpp_newNodeNoArgs()