import noder.tests.base_tree as test_in_cpp


def test_cpp_base_new_base_dimensions():
    return test_in_cpp.test_base_new_base_dimensions()


def test_cpp_base_aggregates_zones():
    return test_in_cpp.test_base_aggregates_zones()


def test_cpp_tree_aggregates_bases_and_zones():
    return test_in_cpp.test_tree_aggregates_bases_and_zones()


def test_cpp_new_base_rejects_incoherent_dimensions():
    return test_in_cpp.test_new_base_rejects_incoherent_dimensions()


def test_cpp_newZoneFromArrays():
    return test_in_cpp.test_newZoneFromArrays()
