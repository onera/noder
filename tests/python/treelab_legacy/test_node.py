import os
import pprint
import numpy as np
import pytest

from tests.python.treelab_legacy import legacy_cgns as cgns


skip_unported = cgns.skip_unported


def test_init1():
    node = cgns.Node()
    assert node.name() == "Node"
    assert node.data() is None
    assert node.children() == []
    assert node.type() == "DataArray_t"

def test_init2():
    node = cgns.Node(Name='MyName', Value=[1,2,3,4], Type='DataArray')
    assert node.name() == "MyName"
    assert np.array_equal(node.numpy(), np.array([1, 2, 3, 4]))
    assert node.type() == "DataArray_t"

def test_init3():
    node = cgns.Node( ['name', np.array([0,1,2]), [], 'DataArray_t'] )
    assert node.name() == "name"
    assert np.array_equal(node.numpy(), np.array([0, 1, 2]))
    assert node.type() == "DataArray_t"

def test_cast():
    node = cgns.castNode( ['name', np.array([0,1,2]), [], 'DataArray_t'] )
    assert node.name() == "name"
    assert np.array_equal(node.numpy(), np.array([0, 1, 2]))
    assert node.type() == "DataArray_t"

def test_parent():
    # create a node and attach it to another node 
    a = cgns.Node( Name='TheParent')
    b = cgns.Node( Name='TheChild', Parent=a )

    p = b.parent()
    assert p is a
    
    p = a.parent()
    assert p is None

def test_path():
    a = cgns.Node( Name='TheParent')
    b = cgns.Node( Name='TheChild', Parent=a )

    path = b.path() 
    assert path == 'TheParent/TheChild'

@skip_unported("treelab Node.save wrapper is intentionally not migrated")
def test_save():
    n = cgns.Node( Name='jamon' )
    n.save('test_node_save.cgns', verbose=False)
    os.unlink('test_node_save.cgns')

def test_name():
    n = cgns.Node( Name='tortilla' )
    assert n.name() == 'tortilla'

def test_value1():
    node = cgns.Node( Value='jamon' )
    value_str = node.value() # will return a readable str
    assert value_str == 'jamon'

def test_value2():
    node = cgns.Node( Value='jamon tortilla croquetas' )

    value_str = node.value() # will return a readable list of str
    assert value_str ==  ['jamon', 'tortilla', 'croquetas']

def test_value3():
    # create a multi-dimensional array to be attributed to several nodes
    # it is very important to set order='F' !
    array = np.array( [[0,1,2],
                        [3,4,5],
                        [6,7,8]], order='F' )

    # create our two nodes, and attribute their values to our array
    jamon    = cgns.Node( Name='jamon', Value=array )
    tortilla = cgns.Node( Name='tortilla', Value=array )

    # get the value of jamon
    jamon_value = jamon.value() # in this case, the same as jamon[1]

    # get a flattened view of the array of node tortilla
    tortilla_value = tortilla.value(ravel=True)


    # our arrays share memory
    assert np.shares_memory(tortilla_value, jamon_value)
    assert np.shares_memory(tortilla_value, array)

    # hence we can modify it in different fashions, all changes will be propagated
    tortilla_value[0] = 12
    array[1,:] = -2

    assert str(tortilla_value) == "[12 -2  6  1 -2  7  2 -2  8]"

def test_setParameters(filename=''):
    t = cgns.Node(Name='main')
    
    def f(x,y): return x+y
    
    t.setParameters('Parameters',
        none=None,
        Fun=f,
        EmptyList=[],
        EmptyDict={},
        EmptyTuple=(),
        NumpyArray=np.array([0,1,2]),
        BooleanList=[True,False,False],
        Boolean=True,
        Int=12,
        IntList=[1,2,3,4],
        Float=13.0,
        FloatList=[1.0,2.0,3.0],
        Str='jamon',
        StrList=['croquetas', 'tortilla'],
        Dict={'Str':'paella','Int':12},
        DictOfDict=dict(SecondDict={'Str':'gazpacho','Int':12}),
        ListOfDict=[{'Str':'pescaito','Int':12},
                    {'Str':'calamares','Int':12},
                    {'Str':'morcilla','Int':12}],
        Node=cgns.Node(Name='n1'),
        DictOfNode={'a':cgns.Node(Name='n5'), 'b':cgns.Node(Name='n6')},
        ListOfNode=[cgns.Node(Name='n2'),cgns.Node(Name='n3'),cgns.Node(Name='n4')],
        )
    
    if filename: t.save(filename)

def test_getParameters(filename=''):
    t = cgns.Node(Name='main')

    def f(x,y): return x+y

    set_params = dict(
        none=None,
        Fun=f,
        EmptyList=[],
        EmptyDict={},
        EmptyTuple=(),
        NumpyArray=np.array([0,1,2]),
        BooleanList=[True,False,False],
        Boolean=True,
        Int=12,
        IntList=[1,2,3,4],
        Float=13.0,
        FloatList=[1.0,2.0,3.0],
        Str='jamon',
        StrList=['croquetas', 'tortilla'],
        Dict={'Str':'paella','Int':12},
        DictOfDict=dict(SecondDict={'Str':'gazpacho','Int':12}),
        ListOfDict=[{'Str':'pescaito','Int':12},
                    {'Str':'calamares','Int':12},
                    {'Str':'morcilla','Int':12}],
        Node=cgns.Node(Name='n1'),
        DictOfNode={'a':cgns.Node(Name='n5'), 'b':cgns.Node(Name='n6')},
        ListOfNode=[cgns.Node(Name='n2'),cgns.Node(Name='n3'),cgns.Node(Name='n4')],
        )

    expected="{'Boolean': array([1], dtype=int32),\n"
    expected+=" 'BooleanList': array([1, 0, 0], dtype=int32),\n"
    expected+=" 'Dict': {'Int': array([12], dtype=int32), 'Str': 'paella'},\n"
    expected+=" 'DictOfDict': {'SecondDict': {'Int': array([12], dtype=int32),\n"
    expected+="                               'Str': 'gazpacho'}},\n"
    expected+=" 'DictOfNode': {'a': None, 'b': None},\n"
    expected+=" 'EmptyDict': None,\n"
    expected+=" 'EmptyList': None,\n"
    expected+=" 'EmptyTuple': None,\n"
    expected+=" 'Float': array([13.]),\n"
    expected+=" 'FloatList': array([1., 2., 3.]),\n"
    expected+=" 'Fun': None,\n"
    expected+=" 'Int': array([12], dtype=int32),\n"
    expected+=" 'IntList': array([1, 2, 3, 4], dtype=int32),\n"
    expected+=" 'ListOfDict': [{'Int': array([12], dtype=int32), 'Str': 'pescaito'},\n"
    expected+="                {'Int': array([12], dtype=int32), 'Str': 'calamares'},\n"
    expected+="                {'Int': array([12], dtype=int32), 'Str': 'morcilla'}],\n"
    expected+=" 'ListOfNode': None,\n"
    expected+=" 'Node': None,\n"
    expected+=" 'NumpyArray': array([0, 1, 2]),\n"
    expected+=" 'Str': 'jamon',\n"
    expected+=" 'StrList': ['croquetas', 'tortilla'],\n"
    expected+=" 'none': None}"

    t.setParameters('Parameters', **set_params)
    get_params = t.getParameters('Parameters')
    try:
        assert pprint.pformat(get_params) == expected
    except AssertionError:
        msg = 'parameters are not equivalent:\n'
        msg+= 'expected:\n'
        msg+= expected+'\n'
        msg+= 'got:\n'
        msg+= pprint.pformat(get_params)
        raise ValueError(msg)

def test_getParameters(filename=''):
    t = cgns.Node(Name='main')

    def f(x,y): return x+y

    set_params = dict(
        none=None,
        Fun=f,
        EmptyList=[],
        EmptyDict={},
        EmptyTuple=(),
        NumpyArray=np.array([0,1,2]),
        BooleanList=[True,False,False],
        Boolean=True,
        Int=12,
        IntList=[1,2,3,4],
        Float=13.0,
        FloatList=[1.0,2.0,3.0],
        Str='jamon',
        StrList=['croquetas', 'tortilla'],
        Dict={'Str':'paella','Int':12},
        DictOfDict=dict(SecondDict={'Str':'gazpacho','Int':12}),
        ListOfDict=[{'Str':'pescaito','Int':12},
                    {'Str':'calamares','Int':12},
                    {'Str':'morcilla','Int':12}],
        Node=cgns.Node(Name='n1'),
        DictOfNode={'a':cgns.Node(Name='n5'), 'b':cgns.Node(Name='n6')},
        ListOfNode=[cgns.Node(Name='n2'),cgns.Node(Name='n3'),cgns.Node(Name='n4')],
        )

    expected="{'Boolean': 1,\n"
    expected+=" 'BooleanList': array([1, 0, 0], dtype=int32),\n"
    expected+=" 'Dict': {'Int': 12, 'Str': 'paella'},\n"
    expected+=" 'DictOfDict': {'SecondDict': {'Int': 12, 'Str': 'gazpacho'}},\n"
    expected+=" 'DictOfNode': {'a': None, 'b': None},\n"
    expected+=" 'EmptyDict': None,\n"
    expected+=" 'EmptyList': None,\n"
    expected+=" 'EmptyTuple': None,\n"
    expected+=" 'Float': 13.0,\n"
    expected+=" 'FloatList': array([1., 2., 3.]),\n"
    expected+=" 'Fun': None,\n"
    expected+=" 'Int': 12,\n"
    expected+=" 'IntList': array([1, 2, 3, 4], dtype=int32),\n"
    expected+=" 'ListOfDict': [{'Int': 12, 'Str': 'pescaito'},\n"
    expected+="                {'Int': 12, 'Str': 'calamares'},\n"
    expected+="                {'Int': 12, 'Str': 'morcilla'}],\n"
    expected+=" 'ListOfNode': None,\n"
    expected+=" 'Node': None,\n"
    expected+=" 'NumpyArray': array([0, 1, 2]),\n"
    expected+=" 'Str': 'jamon',\n"
    expected+=" 'StrList': ['croquetas', 'tortilla'],\n"
    expected+=" 'none': None}"

    t.setParameters('Parameters', **set_params)
    get_params = t.getParameters('Parameters', transform_numpy_scalars=True)
    assert get_params["Boolean"] == 1
    assert np.array_equal(get_params["BooleanList"], np.array([1, 0, 0]))
    assert get_params["Dict"]["Str"] == "paella"
    assert get_params["Dict"]["Int"] == 12
    assert get_params["DictOfDict"]["SecondDict"]["Str"] == "gazpacho"
    assert get_params["DictOfNode"] == {"a": None, "b": None}
    assert get_params["EmptyDict"] is None
    assert get_params["EmptyList"] is None
    assert get_params["EmptyTuple"] is None
    assert get_params["Float"] == 13.0
    assert np.array_equal(get_params["FloatList"], np.array([1.0, 2.0, 3.0]))
    assert get_params["Fun"] is None
    assert get_params["Int"] == 12
    assert np.array_equal(get_params["IntList"], np.array([1, 2, 3, 4]))
    assert get_params["ListOfDict"][0]["Str"] == "pescaito"
    assert get_params["ListOfNode"] is None
    assert get_params["Node"] is None
    assert np.array_equal(get_params["NumpyArray"], np.array([0, 1, 2]))
    assert get_params["Str"] == "jamon"
    assert get_params["StrList"] == ["croquetas", "tortilla"]
    assert get_params["none"] is None

def test_remove():
    # create a node and attach it to another node 
    a = cgns.Node( Name='TheParent')
    b = cgns.Node( Name='TheChild', Parent=a )
    b.remove()
    assert a.name() == "TheParent"
    assert a.data() is None
    assert a.children() == []
    assert a.type() == "DataArray_t"

def test_remove_deep_by_add_child():
    a = cgns.Node(Name='a')
    b = cgns.Node(Name='b')
    z1 = cgns.Node(Name='z1', Type="z_t")
    z2 = cgns.Node(Name='z2', Type="z_t")
    a.addChild(b)
    b.addChild(z1)
    b.addChild(z2)

    for z in a.group(Type="z_t"):
        z.remove()

    assert not a.group(Type="z_t")
    assert not b.children()

def test_remove_deep_by_children_constructor():
    z1 = cgns.Node(Name='z1', Type="z_t")
    z2 = cgns.Node(Name='z2', Type="z_t")
    b = cgns.Node(Name='b', Children=[z1,z2])
    a = cgns.Node(Name='a', Children=[b])

    for z in a.group(Type="z_t"):
        z.remove()

    assert not a.group(Type="z_t")
    assert not b.children()

def test_remove_container_after_new_fields():
    zone = cgns.Zone(Name = 'MyZone')
    zone.newFields({f'f0': [0.]}, Container = f'FV1', GridLocation = 'Vertex')
    zone.newFields({f'f0': [1.]}, Container = f'FV2', GridLocation = 'Vertex')
    zone.newFields({f'f1': [0.]}, Container = f'FC1', GridLocation = 'CellCenter')
    zone.newFields({f'f2': [1.]}, Container = f'FC2', GridLocation = 'CellCenter')

    for fs in zone.group(Type="FlowSolution_t"):
        fs.remove()
        assert fs.parent() is None



def test_findAndRemoveNode():
    # create a node and attach it to another node 
    a = cgns.Node( Name='TheParent')
    b = cgns.Node( Name='TheChild', Value=1, Type='DataArray', Parent=a )

    a.findAndRemoveNode(Name='TheChild', Value=1, Type='DataArray', Depth=1)
    assert a.children() == []
    
def test_findAndRemoveNodes():
    # create a node and attach it to another node 
    a = cgns.Node( Name='TheParent')
    b = cgns.Node( Name='TheChild', Value=1, Type='DataArray', Parent=a )

    a.findAndRemoveNodes(Name='TheChild', Value=1, Type='DataArray', Depth=1)
    assert a.children() == []

def test_getPaths():
    parent = cgns.Node( Name='Parent')
    for n in range(2):
        child = cgns.Node( Name=f'Child{n}', Parent=parent)
    for n in range(2):
        cgns.Node( Name=f'GrandChild{n}', Parent=child)
    paths = parent.getPaths()
    assert paths == ['Parent', 'Parent/Child0', 'Parent/Child1', 'Parent/Child1/GrandChild0', 'Parent/Child1/GrandChild1']


def test_merge():
    b1 = cgns.Node(Name='Base')
    z1 = cgns.Node(Name='zone1', Parent=b1)
    cgns.Node(Name='zone2', Parent=b1)

    b2 = b1.copy()
    cgns.Node(Name='child1', Parent=z1)

    z2 = b2.get(Name='zone2')
    cgns.Node(Name='child2', Parent=z2)
    cgns.Node(Name='zone3', Parent=b2)

    b1.merge(b2)
    assert b1.get_children_names() == ["zone1", "zone2", "zone3"]
    assert b1.get(Name="zone1").get_children_names() == ["child1"]
    assert b1.get(Name="zone2").get_children_names() == ["child2"]


def test_zone_path():
    a = cgns.Tree()
    b = cgns.Base()
    c = cgns.Zone()
    b.attachTo(a)
    c.attachTo(b)
    assert c.path() == "CGNSTree/Base/Zone" # https://github.com/Luispain/treelab/issues/14

def test_get_legacy_cgns_node_type():
    # https://github.com/Luispain/treelab/issues/15
    legacy_type = '"int[IndexDimension]"'
    a = cgns.Node()
    b = cgns.Node(Type=legacy_type)
    a.addChild(b)
    c = a.get(Type=legacy_type)
    assert c


def test_get_at_path():
    a = cgns.Node(Name="a")
    b = cgns.Node(Name="b")
    c = cgns.Node(Name="c")
    b.addChild(c)
    a.addChild(b)
    c2 = a.getAtPath("a/b/c")
    assert c is c2


if __name__ == '__main__':
    test_load_from_path_plus_saveThisNodeOnly()
    # test_saveThisNodeOnly()
