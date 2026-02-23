import pytest
import noder.tests.node_factory as test_in_cpp

def test_cpp_newNodeNoArgs(): return test_in_cpp.test_newNodeNoArgs()
def test_cpp_newNodeNoArgsHasNoneData(): return test_in_cpp.test_newNodeNoArgsHasNoneData()
def test_cpp_newNodeOnlyName(): return test_in_cpp.test_newNodeOnlyName()
def test_cpp_newNodeOnlyNameChars(): return test_in_cpp.test_newNodeOnlyNameChars()
def test_cpp_newNodeNameAndType(): return test_in_cpp.test_newNodeNameAndType()
def test_cpp_newNodeNameTypeAndData(): return test_in_cpp.test_newNodeNameTypeAndData()
def test_cpp_newNodeDataString(): return test_in_cpp.test_newNodeDataString()
def test_cpp_newNodeDataStringChars(): return test_in_cpp.test_newNodeDataStringChars()
def test_cpp_newNodeParent(): return test_in_cpp.test_newNodeParent()

if __name__ == '__main__':
    test_cpp_newNodeDataString()