import noder.tests.io as giocpp


def test_cpp_yaml_read(tmp_path):
    node = giocpp.test_read_yaml(str(tmp_path / "test.yaml"))

    assert node.get_at_path("root/integers") is not None
    assert node.get_at_path("root/message").data().extractString() == "hola"


def test_cpp_yaml_links(tmp_path):
    node = giocpp.test_read_yaml_links(str(tmp_path / "links.yaml"))
    link = node.get_at_path("root/target_link")

    assert link is not None
    assert link.has_link_target()
    assert link.link_target_file() == "."
    assert link.link_target_path() == "/root/target"


def test_cpp_yaml_cgns_tree(tmp_path):
    node = giocpp.test_read_yaml_cgns_tree(str(tmp_path / "tree.yaml"))

    assert node.name() == "CGNSTree"
    assert node.type() == "CGNSTree_t"
    assert node.get_at_path("CGNSTree/Base") is not None
