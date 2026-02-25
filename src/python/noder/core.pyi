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
    def all_by_and(self, name: str = '', type: str = '', data: typing.Any = '', depth: int = 100) -> list[Node]:
        """
        get all nodes by and condition using name, type and data recursively (string or scalar)
        """
    def all_by_and_bool(self, name: str = '', type: str = '', data: bool, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_double(self, name: str = '', type: str = '', data: float, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_float(self, name: str = '', type: str = '', data: float, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_int16(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_int32(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_int64(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_int8(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_uint16(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_uint32(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_uint64(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_and_uint8(self, name: str = '', type: str = '', data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by name/type and scalar data recursively (typed overload)
        """
    def all_by_data(self, data: typing.Any, depth: int = 100) -> list[Node]:
        """
        get all nodes by exact data recursively (string or scalar)
        """
    def all_by_data_bool(self, data: bool, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_double(self, data: float, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_float(self, data: float, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_glob(self, data_pattern: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by glob-pattern data recursively
        """
    def all_by_data_int16(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_int32(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_int64(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_int8(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_uint16(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_uint32(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_uint64(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_data_uint8(self, data: int, depth: int = 100) -> list[Node]:
        """
        Get all nodes by scalar data recursively (typed overload)
        """
    def all_by_name(self, name: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by exact name recursively
        """
    def all_by_name_glob(self, name_pattern: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by glob-pattern name recursively
        """
    def all_by_name_regex(self, name_pattern: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by regex-pattern name recursively
        """
    def all_by_type(self, type: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by exact type recursively
        """
    def all_by_type_glob(self, type_pattern: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by glob-pattern type recursively
        """
    def all_by_type_regex(self, type_pattern: str, depth: int = 100) -> list[Node]:
        """
        get all nodes by regex-pattern type recursively
        """
    def by_and(self, name: str = '', type: str = '', data: typing.Any = '', depth: int = 100) -> Node:
        """
        get node by and condition using name, type and data recursively (string or scalar)
        """
    def by_and_bool(self, name: str = '', type: str = '', data: bool, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_double(self, name: str = '', type: str = '', data: float, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_float(self, name: str = '', type: str = '', data: float, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_int16(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_int32(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_int64(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_int8(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_uint16(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_uint32(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_uint64(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_and_uint8(self, name: str = '', type: str = '', data: int, depth: int = 100) -> Node:
        """
        Get node by name/type and scalar data recursively (typed overload)
        """
    def by_data(self, data: typing.Any, depth: int = 100) -> Node:
        """
        get node by exact data recursively (string or scalar)
        """
    def by_data_bool(self, data: bool, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_double(self, data: float, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_float(self, data: float, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_glob(self, data_pattern: str, depth: int = 100) -> Node:
        """
        get node by glob-pattern data recursively
        """
    def by_data_int16(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_int32(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_int64(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_int8(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_uint16(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_uint32(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_uint64(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_data_uint8(self, data: int, depth: int = 100) -> Node:
        """
        Get node by scalar data recursively (typed overload)
        """
    def by_name(self, name: str, depth: int = 100) -> Node:
        """
        get node by exact name recursively
        """
    def by_name_glob(self, name_pattern: str, depth: int = 100) -> Node:
        """
        get node by glob-pattern name recursively
        """
    def by_name_regex(self, name_pattern: str, depth: int = 100) -> Node:
        """
        get node by regex-pattern name recursively
        """
    def by_type(self, type: str, depth: int = 100) -> Node:
        """
        get node by exact type recursively
        """
    def by_type_glob(self, type_pattern: str, depth: int = 100) -> Node:
        """
        get node by glob-pattern type recursively
        """
    def by_type_regex(self, type_pattern: str, depth: int = 100) -> Node:
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
    """
    
    Hierarchical CGNS-like node with typed payload, children and link metadata.
    
    See C++ counterpart: :ref:`cpp-node-class`.
    """
    @typing.overload
    def __add__(self, arg0: Node) -> _NodeGroup:
        ...
    @typing.overload
    def __add__(self, arg0: _NodeGroup) -> _NodeGroup:
        ...
    def __init__(self, name: str, type: str = 'DataArray_t') -> None:
        """
        Construct a node with a name and type.
        
        See C++ counterpart: :ref:`cpp-node-ctor`.
        """
    def __str__(self) -> str:
        ...
    @typing.overload
    def __truediv__(self, arg0: Node) -> Node:
        ...
    @typing.overload
    def __truediv__(self, arg0: _NodeGroup) -> Node:
        ...
    def add_child(self, node: Node, override_sibling_by_name: bool = True, position: int = -1) -> None:
        """
        Add one child node.
        
        See C++ counterpart: :ref:`cpp-node-addchild`.
        """
    def add_children(self, nodes: list[Node], override_sibling_by_name: bool = True) -> None:
        """
        Add multiple children.
        
        See C++ counterpart: :ref:`cpp-node-addchildren`.
        """
    def attach_to(self, node: Node, position: int = -1, override_sibling_by_name: bool = True) -> None:
        """
        Attach this node to another parent.
        
        See C++ counterpart: :ref:`cpp-node-attachto`.
        """
    def children(self) -> list[Node]:
        """
        Return direct children preserving insertion order.
        
        See C++ counterpart: :ref:`cpp-node-children`.
        """
    def clear_link_target(self) -> None:
        """
        Clear link target metadata.
        
        See C++ counterpart: :ref:`cpp-node-clearlinktarget`.
        """
    def copy(self, deep: bool = False) -> Node:
        """
        Copy subtree; deep copy clones payload arrays.
        
        See C++ counterpart: :ref:`cpp-node-copy`.
        """
    def data(self) -> typing.Any:
        """
        Return node payload as a Data-compatible Python object, or None when empty.
        
        See C++ counterpart: :ref:`cpp-node-data`.
        """
    def descendants(self) -> list[Node]:
        """
        Return this node and all descendants in depth-first order.
        
        See C++ counterpart: :ref:`cpp-node-descendants`.
        """
    def detach(self) -> None:
        """
        Detach from current parent.
        
        See C++ counterpart: :ref:`cpp-node-detach`.
        """
    def get_at_path(self, path: str, path_is_relative: bool = False) -> Node:
        """
        Resolve a node by path (absolute by default).
        
        See C++ counterpart: :ref:`cpp-node-getatpath`.
        """
    def get_children_names(self) -> list[str]:
        """
        Return child names in insertion order.
        
        See C++ counterpart: :ref:`cpp-node-getchildrennames`.
        """
    def get_links(self) -> list[tuple[str, str, str, str, int]]:
        """
        Collect all descendant link definitions in CGNS-compatible tuple format.
        
        See C++ counterpart: :ref:`cpp-node-getlinks`.
        """
    def get_parameters(self, container_name: str, transform_numpy_scalars: bool = False) -> typing.Any:
        """
        Read a parameter container and convert it back to Python dict/list/scalar-like objects.
        
        When `transform_numpy_scalars` is True, single-item NumPy arrays are converted to Python scalars.
        
        See C++ counterpart: :ref:`cpp-node-getparameters`.
        """
    def has_children(self) -> bool:
        """
        Whether node has children.
        
        See C++ counterpart: :ref:`cpp-node-haschildren`.
        """
    def has_link_target(self) -> bool:
        """
        Whether link metadata is defined for this node.
        
        See C++ counterpart: :ref:`cpp-node-haslinktarget`.
        """
    def has_siblings(self) -> bool:
        """
        Whether node has siblings excluding self.
        
        See C++ counterpart: :ref:`cpp-node-hassiblings`.
        """
    def level(self) -> int:
        """
        Return depth from root.
        
        See C++ counterpart: :ref:`cpp-node-level`.
        """
    def link_target_file(self) -> str:
        """
        Return link target file.
        
        See C++ counterpart: :ref:`cpp-node-linktargetfile`.
        """
    def link_target_path(self) -> str:
        """
        Return link target path.
        
        See C++ counterpart: :ref:`cpp-node-linktargetpath`.
        """
    def merge(self, node: Node) -> None:
        """
        Merge descendants from another node with the same root name.
        
        See C++ counterpart: :ref:`cpp-node-merge`.
        """
    def name(self) -> str:
        """
        Return node name.
        
        See C++ counterpart: :ref:`cpp-node-name`.
        """
    def parent(self) -> Node:
        """
        Return parent node or None when detached.
        
        See C++ counterpart: :ref:`cpp-node-parent`.
        """
    def path(self) -> str:
        """
        Return full path from root.
        
        See C++ counterpart: :ref:`cpp-node-path`.
        """
    def pick(self) -> Navigation:
        """
        Return the Navigation helper bound to this node.
        
        See C++ counterpart: :ref:`cpp-node-pick`.
        """
    def position(self) -> int:
        """
        Return sibling position.
        
        See C++ counterpart: :ref:`cpp-node-position`.
        """
    def print_tree(self, max_depth: int = 9999, highlighted_path: str = '', depth: int = 0, last_pos: bool = False, markers: str = '') -> str:
        """
        Render subtree as printable tree text.
        
        See C++ counterpart: :ref:`cpp-node-printtree`.
        """
    def reload_node_data(self, filename: str) -> None:
        """
        Reload this node payload from file using this node path.
        
        See C++ counterpart: :ref:`cpp-node-reloadnodedata`.
        """
    def root(self) -> Node:
        """
        Return root ancestor.
        
        See C++ counterpart: :ref:`cpp-node-root`.
        """
    def save_this_node_only(self, filename: str, backend: str = 'hdf5') -> None:
        """
        Persist only this node payload/metadata into an existing file.
        
        See C++ counterpart: :ref:`cpp-node-savethisnodeonly`.
        """
    def set_data(self, arg0: typing.Any) -> None:
        """
        Set node payload from scalar, string, NumPy array, list/tuple (converted via numpy.asarray), or Data.
        
        See C++ counterpart: :ref:`cpp-node-setdata`.
        """
    def set_link_target(self, target_file: str, target_path: str) -> None:
        """
        Set link target metadata.
        
        See C++ counterpart: :ref:`cpp-node-setlinktarget`.
        """
    def set_name(self, arg0: str) -> None:
        """
        Set node name.
        
        See C++ counterpart: :ref:`cpp-node-setname`.
        """
    def set_parameters(self, container_name: str, container_type: str = 'UserDefinedData_t', parameter_type: str = 'DataArray_t', **kwargs) -> Node:
        """
        Populate a parameter container using treelab-like kwargs semantics.
        
        Special handling:
        - `dict` values create nested parameter containers.
        - `list[dict]` values create ordered `_list_.<index>` entries.
        - `None`, callables and Node values are stored as null-like leaves.
        - other values are stored as leaf data arrays/scalars.
        
        See C++ counterpart: :ref:`cpp-node-setparameters`.
        """
    def set_type(self, arg0: str) -> None:
        """
        Set node type.
        
        See C++ counterpart: :ref:`cpp-node-settype`.
        """
    def siblings(self, include_myself: bool = True) -> list[Node]:
        """
        Return siblings, optionally including self.
        
        See C++ counterpart: :ref:`cpp-node-siblings`.
        """
    def swap(self, node: Node) -> None:
        """
        Swap this node with another node.
        
        See C++ counterpart: :ref:`cpp-node-swap`.
        """
    def type(self) -> str:
        """
        Return node type.
        
        See C++ counterpart: :ref:`cpp-node-type`.
        """
    def write(self, arg0: str) -> None:
        """
        Write this subtree to file.
        
        See C++ counterpart: :ref:`cpp-node-write`.
        """
class _NodeGroup:
    @typing.overload
    def __add__(self, arg0: Node) -> _NodeGroup:
        ...
    @typing.overload
    def __add__(self, arg0: _NodeGroup) -> _NodeGroup:
        ...
    def nodes(self) -> list[Node]:
        ...
def new_node(name: str = '', type: str = '', data: typing.Any = None, parent: Node = None) -> Node:
    """
    Construct a Node and optionally attach it to a parent.
    
    See C++ class: :ref:`cpp-node-class`.
    """
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
