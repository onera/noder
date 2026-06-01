"""

Input-output helpers.

See C++ counterpart: :ref:`cpp-io-module`.
"""
from __future__ import annotations
import numpy
__all__: list[str] = ['ENABLE_HDF5_IO', 'read', 'read_numpy', 'write_numpy']
def read(filename: str, order: str = 'C') -> Node:
    """
    Read a Node hierarchy from file.
    
    The input format is inferred from the filename extension.
    
    Parameters
    ----------
    filename : str
        Input file path.
    order : str, optional
        Memory order of arrays when read (``"C"`` or ``"F"``). Defaults to ``"C"`` (C/NumPy standard).
    
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
