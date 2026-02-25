from __future__ import annotations
import numpy
import typing
from . import factory
from . import io
__all__: list[str] = ['Array', 'Data', 'Navigation', 'Node', 'factory', 'io', 'new_node', 'nodeToPyCGNS', 'pyCGNSToNode', 'registerDefaultFactory']
class Array(Data):
    """
    
    NumPy-backed concrete :py:class:`noder.core.Data` payload.
    
    `Array` stores either numeric values or string-like values and exposes
    lightweight helpers for shape, indexing, contiguity, and printing.
    
    See C++ counterpart: :ref:`cpp-array-class`.
    """
    def __getitem__(self, arg0: typing.Any) -> Array:
        """
        Return a sliced/indexed view as an :py:class:`Array`.
        
        Parameters
        ----------
        key : Any
            NumPy-compatible index or slice expression.
        
        Returns
        -------
        Array
            Indexed value wrapped as Array.
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Construct an empty (None-like) array payload.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/test_array.py
           :language: python
           :pyobject: test_constructorEmpty
        """
    @typing.overload
    def __init__(self, arg0: numpy.ndarray) -> None:
        """
        Construct an Array from an existing NumPy ndarray.
        
        Parameters
        ----------
        arg0 : numpy.ndarray
            Input array to wrap.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/test_array.py
           :language: python
           :pyobject: test_constructorPyArray
        """
    @typing.overload
    def __init__(self, arg0: None) -> None:
        """
        Construct an empty Array from ``None``.
        """
    @typing.overload
    def __init__(self, arg0: str) -> None:
        """
        Construct a string Array from UTF-8 text.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/test_array.py
           :language: python
           :pyobject: test_constructorString
        """
    @typing.overload
    def __init__(self, arg0: Array) -> None:
        """
        Copy-construct from another Array.
        """
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
        """
        Return ``repr(array)`` style representation.
        """
    def __setitem__(self, arg0: typing.Any, arg1: typing.Any) -> None:
        """
        Assign a value into the underlying NumPy array.
        
        Parameters
        ----------
        key : Any
            NumPy-compatible index or slice.
        value : Any
            Value assigned to the selected position(s).
        """
    def dimensions(self) -> int:
        """
        Return array rank (number of dimensions).
        
        Returns
        -------
        int
            Number of dimensions.
        """
    def extractString(self) -> str:
        """
        Decode and return string payload.
        
        Returns
        -------
        str
            UTF-8 string value.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/factory/test_strings.py
           :language: python
           :pyobject: test_arrayFromString
        """
    def getFlatIndex(self, arg0: list[int]) -> int:
        """
        Convert multidimensional indices to one flat index.
        
        Parameters
        ----------
        arg0 : list[int]
            Indices per dimension.
        
        Returns
        -------
        int
            Flat index in the current memory order.
        """
    def getItemAtIndex(self, arg0: int) -> typing.Any:
        """
        Read one element by flat index and return a Python scalar.
        
        Parameters
        ----------
        flatIndex : int
            Zero-based flat index.
        
        Returns
        -------
        Any
            Scalar value at requested position.
        """
    def getPrintString(self, arg0: int) -> str:
        """
        Return a compact textual representation.
        
        Parameters
        ----------
        arg0 : int
            Maximum characters in output.
        
        Returns
        -------
        str
            Formatted representation.
        """
    def getPyArray(self) -> numpy.ndarray:
        """
        Expose the underlying ``numpy.ndarray`` view.
        
        Returns
        -------
        numpy.ndarray
            Wrapped NumPy array.
        """
    def hasString(self) -> bool:
        """
        Check whether this Array stores string data.
        
        Returns
        -------
        bool
            ``True`` for string payloads.
        """
    def info(self) -> str:
        """
        Return detailed metadata and payload summary.
        
        Returns
        -------
        str
            Human-readable Array description.
        """
    def isContiguous(self) -> bool:
        """
        Check whether memory layout is contiguous (C or Fortran order).
        
        Returns
        -------
        bool
            ``True`` when contiguous.
        """
    def isContiguousInStyleC(self) -> bool:
        """
        Check C-order (row-major) contiguity.
        
        Returns
        -------
        bool
            ``True`` for C-order contiguous arrays.
        """
    def isContiguousInStyleFortran(self) -> bool:
        """
        Check Fortran-order (column-major) contiguity.
        
        Returns
        -------
        bool
            ``True`` for Fortran-order contiguous arrays.
        """
    def isNone(self) -> bool:
        """
        Check whether this Array is empty (None-like).
        
        Returns
        -------
        bool
            ``True`` when empty.
        """
    def isScalar(self) -> bool:
        """
        Check whether this Array stores one scalar numeric value.
        
        Returns
        -------
        bool
            ``True`` for scalar payloads.
        """
    def print(self, arg0: int) -> None:
        """
        Print a compact textual representation.
        
        Parameters
        ----------
        arg0 : int
            Maximum characters to emit.
        """
    def shape(self) -> list[int]:
        """
        Return shape as a list of dimension lengths.
        
        Returns
        -------
        list[int]
            Shape vector.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/test_array.py
           :language: python
           :pyobject: test_getArrayProperties
        """
    def size(self) -> int:
        """
        Return total number of elements.
        
        Returns
        -------
        int
            Flat element count.
        """
    def strides(self) -> list[int]:
        """
        Return strides (in bytes) for each dimension.
        
        Returns
        -------
        list[int]
            Stride vector.
        """
class Data:
    """
    
    Abstract payload interface used by :py:class:`noder.core.Node`.
    
    This class is normally returned by :py:meth:`noder.core.Node.data` and most
    concrete payloads are :py:class:`noder.core.Array`.
    
    See C++ counterpart: :ref:`cpp-data-class`.
    """
    def extractString(self) -> str:
        """
        Extract the payload as a Python string.
        
        Returns
        -------
        str
            UTF-8 string representation of the payload.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/factory/test_strings.py
           :language: python
           :pyobject: test_arrayFromString
        """
    def hasString(self) -> bool:
        """
        Check whether this payload represents a string-like value.
        
        Returns
        -------
        bool
            ``True`` when the payload stores string-compatible data.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/node/test_node.py
           :language: python
           :pyobject: test_dataInt
        """
    def isNone(self) -> bool:
        """
        Check whether this payload is empty (None-like).
        
        Returns
        -------
        bool
            ``True`` when payload is considered empty.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/node/test_node.py
           :language: python
           :pyobject: test_parent_empty
        """
    def isScalar(self) -> bool:
        """
        Check whether this payload is a scalar numeric value.
        
        Returns
        -------
        bool
            ``True`` when payload corresponds to a scalar.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/array/test_array.py
           :language: python
           :pyobject: test_isScalar
        """
class Navigation:
    """
    
    Tree-query helper attached to :py:class:`noder.core.Node` via :py:meth:`Node.pick`.
    
    Navigation methods search descendants by name, type, data, or combined predicates.
    
    See C++ counterpart: :ref:`cpp-navigation-class`.
    """
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
        """
        Create a :py:class:`_NodeGroup` from two nodes.
        
        Example
        -------
        .. literalinclude:: ../../../tests/python/node/test_node_group.py
           :language: python
           :pyobject: test_operator_plus_attach_to_first_parent
        """
    @typing.overload
    def __add__(self, arg0: _NodeGroup) -> _NodeGroup:
        """
        Prefix a node before an existing :py:class:`_NodeGroup`.
        """
    def __init__(self, name: str, type: str = 'DataArray_t') -> None:
        """
        Construct a node with a name and type.
        
        See C++ counterpart: :ref:`cpp-node-ctor`.
        """
    def __str__(self) -> str:
        ...
    @typing.overload
    def __truediv__(self, arg0: Node) -> Node:
        """
        Attach ``rhs`` as child of ``lhs`` and return ``rhs``.
        """
    @typing.overload
    def __truediv__(self, arg0: _NodeGroup) -> Node:
        """
        Attach all nodes in ``rhs`` as children of ``lhs`` and return the last node.
        """
    def add_child(self, node: Node, override_sibling_by_name: bool = True, position: int = -1) -> None:
        """
        Add one child node.
        
        See C++ counterpart: :ref:`cpp-node-addchild`.
        
        Example
        -------
        The following example is imported dynamically from the test suite:
        
        .. literalinclude:: ../../../tests/python/node/test_node.py
           :language: python
           :start-after: # docs:start add_child_example
           :end-before: # docs:end add_child_example
           :dedent: 4
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
    """
    
    Internal helper type returned by Node expression operators.
    
    This type groups nodes for chained expressions such as ``a / (b + c)``.
    """
    @typing.overload
    def __add__(self, arg0: Node) -> _NodeGroup:
        """
        Append a node to this group and return a new group.
        """
    @typing.overload
    def __add__(self, arg0: _NodeGroup) -> _NodeGroup:
        """
        Concatenate two node groups.
        """
    def nodes(self) -> list[Node]:
        """
        Return grouped nodes in insertion order.
        
        Returns
        -------
        list[Node]
            Grouped nodes.
        """
def new_node(name: str = '', type: str = '', data: typing.Any = None, parent: Node = None) -> Node:
    """
    Construct a Node and optionally attach it to a parent.
    
    Parameters
    ----------
    name : str, optional
        Node name.
    type : str, optional
        Node type.
    data : Any, optional
        Payload convertible to :py:class:`noder.core.Data`.
    parent : Node or None, optional
        Parent node for immediate attachment.
    
    Returns
    -------
    Node
        Newly created node.
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/node/test_node_factory.py
       :language: python
       :pyobject: test_new_node_parent
    
    See C++ class: :ref:`cpp-node-class`.
    """
def nodeToPyCGNS(arg0: Node) -> list:
    """
    Convert a Node hierarchy to a Python CGNS-like nested list.
    
    Parameters
    ----------
    arg0 : Node
        Root node to convert.
    
    Returns
    -------
    list
        CGNS-like list representation.
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
       :language: python
       :pyobject: test_nodeToPyCGNS_tree
    """
def pyCGNSToNode(arg0: list) -> Node:
    """
    Convert a Python CGNS-like nested list into a Node hierarchy.
    
    Parameters
    ----------
    arg0 : list
        CGNS-like list representation.
    
    Returns
    -------
    Node
        Converted root node.
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
       :language: python
       :pyobject: test_pyCGNSToNode_tree
    """
def registerDefaultFactory() -> None:
    """
    Register the default data factory used by new nodes.
    
    Returns
    -------
    None
    """
