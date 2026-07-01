import os
import pprint
import numpy as np
import pytest

from . import legacy_cgns as cgns

def get_cart():
    x, y, z = np.meshgrid( np.linspace(0,1,3),
                           np.linspace(0,1,3),
                           np.linspace(0,1,3), indexing='ij')
    cart = cgns.newZoneFromArrays( 'block', ['x','y','z'],
                                            [ x,  y,  z ])
    return cart

def test_newFields1():
    zone = get_cart()
    f = zone.newFields('field')
    expected_f = np.zeros((3,3,3), dtype=np.float64)
    assert str(f) == str(expected_f)

def test_newFields2():
    zone = get_cart()
    f = zone.newFields('field', Container='FlowSolution#Centers')
    expected_f = np.zeros((2,2,2), dtype=np.float64)
    assert str(f) == str(expected_f)

def test_newFields3():
    zone = get_cart()
    f1, f2 = zone.newFields({'f1':1,'f2':2})
    expected_f1 = np.full((3,3,3), 1, dtype=np.float64)
    assert str(f1) == str(expected_f1)

    expected_f2 = np.full((3,3,3), 2, dtype=np.float64)
    assert str(f2) == str(expected_f2)

def test_boundaries():
    zone = get_cart()
    zone.newFields(['f1','f2'])
    zone.newFields('f3', Container='FlowSolution#Centers')
    boundaries = zone.boundaries()
    
def test_updateShape():
    zone = cgns.Zone(Name='Zone')
    with pytest.raises(Exception):
        zone.updateShape()

    fs = cgns.Node(Name='FlowSolution', Type='FlowSolution', Parent=zone)
    data = cgns.Node(Name='Data', Type='DataArray', Value=np.ones((3,2,5), order='F'), Parent=fs)

    zone.updateShape()
    assert np.array_equal(zone.value(), np.array([[3,2,0],[2,1,0],[5,4,0]], order='F'))

    data.setValue(np.arange(10))
    zone.updateShape()
    assert np.array_equal(zone.value(), np.array([[10,9,0]], order='F'))

    other_data = cgns.Node(Name='OtherData', Type='DataArray', Value=np.ones((3,9), order='F'), Parent=fs)
    with pytest.raises(Exception):
        zone.updateShape()
    other_data.remove()

    zone = cgns.Zone(Name='Zone', Children=[fs]) # zone shape is set in __init__
    assert np.array_equal(zone.value(), np.array([[10,9,0]], order='F'))

def test_get_zones():
    t = cgns.Tree()
    b = cgns.Base()
    z1 = cgns.Zone()
    z2 = cgns.Zone()
    z3 = cgns.Zone()
    t.addChild(b)
    b.addChildren([z1,z2,z3], override_sibling_by_name=False)
    
    assert len(t.zones()) == 3
    assert len(b.zones()) == 3

def test_remove_containers_in_zone():
    fs1 = cgns.Node(Name="fs1", Type="FlowSolution_t")
    zone = cgns.Zone(Name='MyZone')
    zone.addChild(fs1)

    for fs in zone.group(Type="FlowSolution_t"):
        fs.remove()

    assert len(zone.group(Type="FlowSolution_t")) == 0
