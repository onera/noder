import pytest
import noder.tests.learn_pybind11 as test_in_cpp

def test_slicingReferences(): return test_in_cpp.slicingReferences()

def test_pointerAccess(): return test_in_cpp.pointerAccess()
