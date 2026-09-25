"""

Input-output helpers.

See C++ counterpart: :ref:`cpp-io-module`.
"""
from __future__ import annotations
from noder.core import Node
import numpy
import typing
__all__: list[str] = ['ENABLE_HDF5_IO', 'LazyHdf5Reader', 'Node', 'read', 'read_numpy', 'write_numpy']
class LazyHdf5Reader:
    """

    Lazy metadata-first reader for CGNS/HDF5 files.

    The returned root is a regular Node. Direct children and payloads are loaded
    on demand, so ``loaded_children()`` can be used for paged terminal views.
    """
    def __init__(self, filename: str, order: str = 'F') -> None:
        ...
    def close(self) -> None:
        ...
    def ensure_children_loaded(self, node: Node, minimum_children: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    def ensure_data_loaded(self, node: Node) -> None:
        ...
    def filename(self) -> str:
        ...
    def is_open(self) -> bool:
        ...
    def order(self) -> str:
        ...
    def root(self) -> Node:
        ...
def read(filename: str, order: str = 'F') -> Node:
    """
    Read a Node hierarchy from file.
    
    The input format is inferred from the filename extension.
    
    Parameters
    ----------
    filename : str
        Input file path.
    order : str, optional
        Memory order of arrays when read (``"C"`` or ``"F"``). Defaults to ``"F"`` (CGNS/Fortran convention).
    
    Returns
    -------
    Node
        Root node read from disk.
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/io/test_io.py
       :language: python
       :pyobject: test_read
    """
def read_numpy(filename: str, dataset_name: str = 'numpy', order: str = 'F') -> numpy.ndarray:
    """
    Read a NumPy array from an HDF5 dataset.
    
    Parameters
    ----------
    filename : str
        Input file path.
    dataset_name : str, optional
        Dataset name inside file.
    order : str, optional
        Memory order of returned array (``"C"`` or ``"F"``).
    
    Returns
    -------
    numpy.ndarray
        Loaded array.
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/io/test_io.py
       :language: python
       :pyobject: test_write_and_read_numerical_numpy
    """
def write_numpy(array: numpy.ndarray, filename: str, dataset_name: str = 'numpy') -> None:
    """
    Write a NumPy array to a dataset in an HDF5 file.
    
    Parameters
    ----------
    array : numpy.ndarray
        Array to persist.
    filename : str
        Output file path.
    dataset_name : str, optional
        Dataset name inside file.
    
    Returns
    -------
    None
    
    Example
    -------
    .. literalinclude:: ../../../tests/python/io/test_io.py
       :language: python
       :pyobject: test_write_and_read_numerical_numpy
    """
ENABLE_HDF5_IO: bool = True
