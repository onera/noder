"""
Copyright (c) 2026 ONERA. All rights reserved.

noder: Node-Oriented Data Extraction & Representation
"""
from __future__ import annotations
import typing
import glob as glob
from noder._cgns import Base
from noder._cgns import Tree
from noder._cgns import Zone
from noder._cgns import new_base
from noder._cgns import new_zone_from_arrays
from noder.cgns import add
from noder.cgns import merge
from noder.cgns import new_tree
from noder.cgns import new_zone_from_dict
from noder.core import Array
from noder.core import Node
from noder.core import factory
from noder.core import io as _core_io
from noder.core.io import read
from noder.core.io import read_numpy
from noder.core.io import write_numpy
from noder.core import new_node
from noder.core import nodeToPyCGNS
from noder.core import pyCGNSToNode
from noder.core import registerDefaultFactory
import os as os
from pathlib import Path
import sys as sys
from . import _version
from . import core
from . import _cgns
from . import cgns
__all__: list = ['__version__', 'get_package_dir', 'get_include_dir', 'get_cmake_dir', 'Node', 'Array', 'Tree', 'Base', 'Zone', 'factory', 'registerDefaultFactory', 'new_node', 'zeros', 'new_base', 'new_tree', 'new_zone_from_arrays', 'new_zone_from_dict', 'add', 'merge', 'read', 'read_from_path', 'write_numpy', 'read_numpy', 'pyCGNSToNode', 'nodeToPyCGNS']
def get_cmake_dir() -> str:
    ...
def get_include_dir() -> str:
    ...
def get_package_dir() -> str:
    ...
def read_from_path(
    filename: str | Path,
    path: str,
    order: str = "F",
    safe_mode: bool = False,
    strict_types: bool = True,
) -> Node | None:
    """
    Read and return one node from a file.

        For HDF5/CGNS files, this uses :class:`LazyHdf5Reader` and reads metadata
        only for the nodes along the requested path. Sibling metadata and payload
        arrays remain untouched.
        The returned node is detached from the file root and keeps its lazy file
        state alive until the node is released or its expansion is detached.

        YAML files use the regular eager reader because Noder has no lazy YAML
        reader. Missing paths return ``None``.

        Parameters
        ----------
        filename : str or pathlib.Path
            Input file path. The file format is inferred from its extension.
        path : str
            Slash-separated path relative to the file root.
        order : str, optional
            Memory order of arrays when read (``"C"`` or ``"F"``). Defaults to
            ``"F"`` (CGNS/Fortran convention).
        safe_mode : bool, optional
            Replace malformed HDF5/CGNS nodes by ``Corrupted_t`` markers instead
            of raising. Defaults to ``False``.
        strict_types : bool, optional
            Promote I1/I2 to I4, U1/U2 to U4, and B1 to I4 for CGNS-compatible
            payload types. Defaults to ``True``.

        Returns
        -------
        Node or None
            The selected node, detached from its original parent, or ``None`` if
            the path does not exist.

    """
def zeros(*args: typing.Any, **kwargs: typing.Any) -> Array:
    ...
__version__: str = '1.5.12.dev1+g013aca72d.d20261110'
