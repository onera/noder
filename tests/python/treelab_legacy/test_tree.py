from . import legacy_cgns as cgns

def test_remove_containers_after_tree_creation():
    fs1 = cgns.Node(Name="fs1", Type="FlowSolution_t")
    zone = cgns.Zone(Name='MyZone')
    zone.addChild(fs1)

    t = cgns.Tree(MyBase=zone)

    for fs in t.group(Type="FlowSolution_t"):
        fs.remove()

    assert len(t.group(Type="FlowSolution_t")) == 0
