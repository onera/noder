import noder.tests.navigation as test_in_cpp

def test_cpp_get_childByName(): return test_in_cpp.childByName()

def test_cpp_get_byName(): return test_in_cpp.byName()

def test_cpp_get_byNamePattern(): return test_in_cpp.byNamePattern()

def test_cpp_get_childByType(): return test_in_cpp.childByType()

def test_cpp_get_byType(): return test_in_cpp.byType()

def test_cpp_get_byTypePattern(): return test_in_cpp.byTypePattern()

def test_cpp_get_childByData(): return test_in_cpp.childByData()

def test_cpp_get_childByDataUsingChar(): return test_in_cpp.childByDataUsingChar()

def test_cpp_get_childByDataScalarDirect(): return test_in_cpp.childByDataScalarDirect()

if __name__ == '__main__':
    test_cpp_get_childByName()
