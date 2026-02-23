import noder.tests.navigation as test_in_cpp

def test_cpp_get_childByName(): return test_in_cpp.childByName()

def test_cpp_get_byName(): return test_in_cpp.byName()
def test_cpp_get_allByName(): return test_in_cpp.allByName()

def test_cpp_get_byNameRegex(): return test_in_cpp.byNameRegex()
def test_cpp_get_allByNameRegex(): return test_in_cpp.allByNameRegex()

def test_cpp_get_byNameGlob(): return test_in_cpp.byNameGlob()
def test_cpp_get_allByNameGlob(): return test_in_cpp.allByNameGlob()

def test_cpp_get_childByType(): return test_in_cpp.childByType()

def test_cpp_get_byType(): return test_in_cpp.byType()
def test_cpp_get_allByType(): return test_in_cpp.allByType()

def test_cpp_get_byTypeRegex(): return test_in_cpp.byTypeRegex()
def test_cpp_get_allByTypeRegex(): return test_in_cpp.allByTypeRegex()

def test_cpp_get_byTypeGlob(): return test_in_cpp.byTypeGlob()
def test_cpp_get_allByTypeGlob(): return test_in_cpp.allByTypeGlob()

def test_cpp_get_childByData(): return test_in_cpp.childByData()

def test_cpp_get_childByDataUsingChar(): return test_in_cpp.childByDataUsingChar()

def test_cpp_get_childByDataScalarDirect(): return test_in_cpp.childByDataScalarDirect()

def test_cpp_get_byData(): return test_in_cpp.byData()
def test_cpp_get_allByData(): return test_in_cpp.allByData()

def test_cpp_get_byDataGlob(): return test_in_cpp.byDataGlob()
def test_cpp_get_allByDataGlob(): return test_in_cpp.allByDataGlob()

def test_cpp_get_byDataScalarDirect(): return test_in_cpp.byDataScalarDirect()
def test_cpp_get_allByDataScalarDirect(): return test_in_cpp.allByDataScalarDirect()

def test_cpp_get_byAnd(): return test_in_cpp.byAnd()
def test_cpp_get_byAndScalar(): return test_in_cpp.byAndScalar()
def test_cpp_get_allByAnd(): return test_in_cpp.allByAnd()

if __name__ == '__main__':
    test_cpp_get_childByName()
