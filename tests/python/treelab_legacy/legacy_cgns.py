import numpy as np
import pytest

from noder import add as _noder_add
from noder import merge, new_zone_from_arrays, read
from noder.cgns import Base as _Base
from noder.cgns import Tree as _Tree
from noder.cgns import Zone as _Zone
from noder.core import Node as _Node
from noder.core import nodeToPyCGNS, pyCGNSToNode


def _normalize_type(type_name):
    if type_name is None:
        return "DataArray_t"
    if type_name.endswith("_t") or type_name.startswith('"'):
        return type_name
    return f"{type_name}_t"


def _scalarize(value):
    if isinstance(value, np.ndarray) and value.size == 1:
        return value.item()
    return value


def _decode_bytes(value):
    if isinstance(value, bytes):
        return value.decode()
    if isinstance(value, np.ndarray) and value.dtype.kind in "SU":
        decoded = [_decode_bytes(item) for item in value.tolist()]
        return decoded[0] if len(decoded) == 1 else decoded
    if isinstance(value, dict):
        return {key: _decode_bytes(child) for key, child in value.items()}
    if isinstance(value, list):
        return [_decode_bytes(child) for child in value]
    return value


def _normalize_field_values(input_fields):
    if isinstance(input_fields, dict):
        return {
            key: value[0] if isinstance(value, (list, tuple)) and len(value) == 1 else value
            for key, value in input_fields.items()
        }
    return input_fields


def _legacy_get(self, Name="", Type="", Value="", Depth=100, **kwargs):
    name = kwargs.pop("name", Name)
    type_name = kwargs.pop("type", kwargs.pop("Type", Type))
    data = kwargs.pop("data", kwargs.pop("Data", Value))
    depth = kwargs.pop("depth", kwargs.pop("Depth", Depth))
    type_name = _normalize_type(type_name) if type_name else ""
    if isinstance(data, str):
        return self.pick().by_and_glob(name, type_name, data, depth)
    return self.pick().by_and(name, type_name, data, depth)


def _legacy_group(self, Name="", Type="", Value="", Depth=100, **kwargs):
    name = kwargs.pop("name", Name)
    type_name = kwargs.pop("type", kwargs.pop("Type", Type))
    data = kwargs.pop("data", kwargs.pop("Data", Value))
    depth = kwargs.pop("depth", kwargs.pop("Depth", Depth))
    type_name = _normalize_type(type_name) if type_name else ""
    if isinstance(data, str):
        return self.pick().all_by_and_glob(name, type_name, data, depth)
    return self.pick().all_by_and(name, type_name, data, depth)


class _LegacyMixin:
    def value(self, ravel=False):
        value = self.interpret_data()
        if ravel and isinstance(value, np.ndarray):
            return value.ravel(order="K")
        return value

    def setValue(self, value):
        self.set_data(value)

    def addChild(self, node, override_sibling_by_name=True):
        self.add_child(node, override_sibling_by_name=override_sibling_by_name)

    def addChildren(self, nodes, override_sibling_by_name=True):
        self.add_children(nodes, override_sibling_by_name=override_sibling_by_name)

    def attachTo(self, parent, position=-1, override_sibling_by_name=True):
        self.attach_to(parent, position=position, override_sibling_by_name=override_sibling_by_name)

    def remove(self):
        self.detach()

    def get(self, *args, **kwargs):
        if args:
            kwargs.setdefault("Name", args[0])
        return _legacy_get(self, **kwargs)

    def group(self, *args, **kwargs):
        if args:
            kwargs.setdefault("Name", args[0])
        return _legacy_group(self, **kwargs)

    def getAtPath(self, path):
        return self.get_at_path(path)

    def getPaths(self):
        return [node.path() for node in self.descendants()]

    def setParameters(self, container_name, **kwargs):
        return self.set_parameters(container_name, **kwargs)

    def getParameters(self, container_name, transform_numpy_scalars=False):
        return _decode_bytes(
            self.get_parameters(container_name, transform_numpy_scalars=transform_numpy_scalars)
        )

    def findAndRemoveNode(self, **kwargs):
        node = self.get(**kwargs)
        if node is not None:
            node.detach()

    def findAndRemoveNodes(self, **kwargs):
        for node in list(self.group(**kwargs)):
            node.detach()

    def save(self, filename, **kwargs):
        self.write(filename)

    def saveThisNodeOnly(self, filename, **kwargs):
        self.save_this_node_only(filename)

    def copy(self, deep=False):
        return castNode(nodeToPyCGNS(super().copy(deep=deep)))


class Node(_LegacyMixin, _Node):
    def __init__(
        self,
        *args,
        Name="Node",
        Value=None,
        Type="DataArray_t",
        Parent=None,
        Children=None,
    ):
        if args:
            if len(args) != 1 or not isinstance(args[0], list):
                raise TypeError("legacy Node accepts at most one CGNS-like list argument")
            Name, Value, Children, Type = args[0]

        super().__init__(Name, _normalize_type(Type))
        if Value is not None:
            self.set_data(Value)
        if Children:
            self.add_children([castNode(child) for child in Children], override_sibling_by_name=False)
        if Parent is not None:
            self.attach_to(Parent)


class Zone(_LegacyMixin, _Zone):
    def __init__(self, Name="Zone", Value=None, Parent=None, Children=None):
        super().__init__(Name)
        if Value is not None:
            self.set_data(Value)
        if Children:
            self.add_children([castNode(child) for child in Children], override_sibling_by_name=False)
            try:
                self.update_shape()
            except Exception:
                pass
        if Parent is not None:
            self.attach_to(Parent)

    def newFields(self, input_fields, Container="FlowSolution", GridLocation="auto", **kwargs):
        return self.new_fields(
            _normalize_field_values(input_fields),
            container=Container,
            grid_location=GridLocation,
            **kwargs,
        )

    def updateShape(self):
        return self.update_shape()

    def inferLocation(self, container):
        return self.infer_location(container)


class Base(_LegacyMixin, _Base):
    def __init__(self, Name="Base", Value=None, Parent=None, Children=None):
        super().__init__(Name)
        if Value is not None:
            self.set_data(Value)
        if Children:
            self.add_children([castNode(child) for child in Children], override_sibling_by_name=False)
        if Parent is not None:
            self.attach_to(Parent)


class Tree(_LegacyMixin, _Tree):
    def __init__(self, **bases):
        super().__init__()
        if bases:
            built = _noder_add(**bases)
            for base in built.bases():
                self.add_child(base, override_sibling_by_name=False)


def castNode(value):
    if isinstance(value, _Node):
        return value
    if not isinstance(value, list):
        return pyCGNSToNode(value)

    node_type = value[3]
    kwargs = {
        "Name": value[0],
        "Value": value[1],
        "Children": value[2],
    }
    if node_type == "CGNSTree_t":
        node = Tree()
        node.add_children([castNode(child) for child in value[2]], override_sibling_by_name=False)
        return node
    if node_type == "CGNSBase_t":
        return Base(**kwargs)
    if node_type == "Zone_t":
        return Zone(**kwargs)
    kwargs["Type"] = node_type
    return Node(**kwargs)


def newZoneFromArrays(name, array_names, arrays):
    native = new_zone_from_arrays(name, array_names, arrays)
    zone = Zone(Name=name)
    if native.data() is not None:
        zone.set_data(native.data())
    zone.add_children(native.children(), override_sibling_by_name=False)
    return zone


def add(*data1, **data2):
    built = _noder_add(*data1, **data2)
    tree = Tree()
    for base in built.bases():
        tree.add_child(base, override_sibling_by_name=False)
    return tree


def check_only_contains_node_instances(node):
    return isinstance(node, _Node) and all(isinstance(child, _Node) for child in node.children())


def load(filename):
    return read(filename)


def load_workflow_parameters(filename):
    tree = read(filename)
    return tree.get_parameters("WorkflowParameters", transform_numpy_scalars=True)


def load_from_path(filename, path):
    return read(filename).get_at_path(path)


def skip_unported(reason):
    return pytest.mark.skip(reason=f"treelab legacy test skipped: {reason}")


if not hasattr(_Node, "remove"):
    _Node.remove = lambda self: self.detach()
