import pytest

from tests.python.treelab_legacy import legacy_cgns as cgns


skip_unported = cgns.skip_unported

def test_check_only_contains_node_instances_true():
    a = cgns.Node()
    b = cgns.Node(Parent=a)
    c = cgns.Node(Parent=a)

    assert cgns.check_only_contains_node_instances(a)


def test_add_from_list():
    a = cgns.Tree()
    b = cgns.Base(Name='BaseA')
    c = cgns.Zone(Name='ZoneC')
    e = cgns.Zone(Name='ZoneE')
    f = cgns.Zone(Name='ZoneF')


    d = cgns.add([a,b,[c,[e,f]]])
    assert len(d.zones()) == 3
    assert len(d.bases()) == 1

def test_add_from_kwargs():
    c = cgns.Zone(Name='ZoneC')
    e = cgns.Zone(Name='ZoneE')
    f = cgns.Zone(Name='ZoneF')


    d = cgns.add(Group1 = [c,e], Group2 = f)

    assert len(d.zones()) == 3
    assert len(d.bases()) == 2


def get_zones_and_concatenate_flow_solution_nodes_at_vertex_and_cell_from_heterogeneous_data(heterogeneous_data):
    assert heterogeneous_data
    t = cgns.add(**heterogeneous_data)
    output_zones = []
    for zone in t.zones():
        concatenate_flow_solution_nodes_at_vertex_and_cell_of_zone(zone)    
        output_zones += [zone]
    return output_zones

def concatenate_flow_solution_nodes_at_vertex_and_cell_of_zone(zone : cgns.Zone):
    assert isinstance(zone, cgns.Zone)

    vertex_fields = {}
    cell_fields   = {}
    for fs in zone.group(Type = 'FlowSolution_t'):
        fields = {n.name(): n.value().copy()
                                        for n in fs.group(Type = 'DataArray_t')}
        if zone.inferLocation  (fs.name()) == 'Vertex':
            vertex_fields.update(fields)
        elif zone.inferLocation(fs.name()) == 'CellCenter':
            cell_fields  .update(fields)
        else: continue

        fs.remove()

        assert fs.parent() is None
        assert not zone.get(Name=fs.name(),Type="FlowSolution_t", Depth=1)

    for array in [[vertex_fields, 'FlowSolution',         'Vertex'],
                  [cell_fields,   'FlowSolution#Centers', 'CellCenter']]:
        fields, Container, GridLocation = array
        if len(fields):
            zone.newFields(fields, Container = Container,
                                                    GridLocation = GridLocation)



def test_remove_containers_after_add():
    fs1 = cgns.Node(Name="fs1", Type="FlowSolution_t")
    zone = cgns.Zone(Name='MyZone')
    zone.addChild(fs1)

    t = cgns.add(MyBase=zone)

    for fs in t.group(Type="FlowSolution_t"):
        fs.remove()

    assert len(t.group(Type="FlowSolution_t")) == 0
