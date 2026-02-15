from __future__ import annotations
import numpy
import typing
from . import factory
from . import io
__all__: list[str] = ['Array', 'Data', 'Navigation', 'Node', 'factory', 'io', 'new_node', 'nodeToPyCGNS', 'pyCGNSToNode', 'registerDefaultFactory']
class Array(Data):
    def __getitem__(self, arg0: typing.Any) -> Array:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: numpy.ndarray) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: None) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: str) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: Array) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: typing.Any, arg1: typing.Any) -> None:
        ...
    def dimensions(self) -> int:
        ...
    def extractString(self) -> str:
        ...
    def getFlatIndex(self, arg0: list[int]) -> int:
        ...
    def getItemAtIndex(self, arg0: int) -> typing.Any:
        ...
    def getPrintString(self, arg0: int) -> str:
        ...
    def getPyArray(self) -> numpy.ndarray:
        ...
    def hasString(self) -> bool:
        ...
    def info(self) -> str:
        ...
    def isContiguous(self) -> bool:
        ...
    def isContiguousInStyleC(self) -> bool:
        ...
    def isContiguousInStyleFortran(self) -> bool:
        ...
    def isNone(self) -> bool:
        ...
    def isScalar(self) -> bool:
        ...
    def print(self, arg0: int) -> None:
        ...
    def shape(self) -> list[int]:
        ...
    def size(self) -> int:
        ...
    def strides(self) -> list[int]:
        ...
class Data:
    def extractString(self) -> str:
        ...
    def hasString(self) -> bool:
        ...
    def isNone(self) -> bool:
        ...
    def isScalar(self) -> bool:
        ...
class Navigation:
    def by_name(self, name: str, depth: int = 100) -> Node:
        """
        get node by exact name recursively
        """
    def by_name_pattern(self, name_pattern: str, depth: int = 100) -> Node:
        """
        get node by regex-pattern name recursively
        """
    def by_type(self, type: str, depth: int = 100) -> Node:
        """
        get node by exact type recursively
        """
    def by_type_pattern(self, type_pattern: str, depth: int = 100) -> Node:
        """
        get node by regex-pattern type recursively
        """
    def child_by_data(self, data: typing.Any) -> Node:
        """
        Get child node by data (string or scalar)
        """
    def child_by_data_bool(self, data: bool) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_double(self, data: float) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_float(self, data: float) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_int16(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_int32(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_int64(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_int8(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_uint16(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_uint32(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_uint64(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_data_uint8(self, data: int) -> Node:
        """
        Get child node by scalar data (typed overload)
        """
    def child_by_name(self, arg0: str) -> Node:
        """
        Get child node by name
        """
    def child_by_type(self, arg0: str) -> Node:
        """
        Get child node by type
        """
class Node:
    def __init__(self, name: str, type: str = 'DataArray_t') -> None:
        """
        Node constructor
        """
    def __str__(self) -> str:
        ...
    def add_child(self, arg0: Node) -> None:
        ...
    def attach_to(self, arg0: Node) -> None:
        ...
    def children(self) -> list[Node]:
        ...
    def data(self) -> typing.Any:
        ...
    def descendants(self) -> list[Node]:
        ...
    def detach(self) -> None:
        ...
    def level(self) -> int:
        ...
    def name(self) -> str:
        ...
    def parent(self) -> Node:
        """
        Returns the parent Node or None if no parent exists.
        """
    def path(self) -> str:
        ...
    def pick(self) -> Navigation:
        ...
    def position(self) -> int:
        ...
    def print_tree(self, max_depth: int = 9999, highlighted_path: str = '', depth: int = 0, last_pos: bool = False, markers: str = '') -> str:
        """
        print node in tree format
        """
    def root(self) -> Node:
        ...
    def set_data(self, arg0: typing.Any) -> None:
        ...
    def set_name(self, arg0: str) -> None:
        ...
    def set_type(self, arg0: str) -> None:
        ...
    def type(self) -> str:
        ...
    def write(self, arg0: str) -> None:
        ...
def new_node(name: str = '', type: str = '', data: typing.Any = False, parent: Node = None) -> Node:
    ...
def nodeToPyCGNS(arg0: Node) -> list:
    """
    Convert a Node to a Python CGNS-like list.
    """
def pyCGNSToNode(arg0: list) -> Node:
    """
    Convert a Python CGNS-like list to a Node.
    """
def registerDefaultFactory() -> None:
    ...
