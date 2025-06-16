import noder.tests.navigation as test_in_cpp

def test_cpp_get_childByName(): return test_in_cpp.childByName()

def test_cpp_get_byName(): return test_in_cpp.byName()

def test_cpp_get_byNamePattern(): return test_in_cpp.byNamePattern()

def test_cpp_get_childByType(): return test_in_cpp.childByType()

def test_cpp_get_byType(): return test_in_cpp.byType()

def test_cpp_get_byTypePattern(): return test_in_cpp.byTypePattern()

if __name__ == '__main__':
    test_cpp_get_childByName()