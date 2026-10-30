"""
Copyright (c) 2026 ONERA. All rights reserved.

noder: Node-Oriented Data Extraction & Representation
"""
import sys
import os
import glob
from pathlib import Path

from ._version import version as __version__

__all__ = [
    "__version__",
    "get_package_dir",
    "get_include_dir",
    "get_cmake_dir",
    # Main classes
    "Node",
    "Array",
    "Tree",
    "Base",
    "Zone",
    # Factory utilities
    "factory",
    "registerDefaultFactory",
    # Convenience functions
    "new_node",
    "zeros",
    "new_base",
    "new_tree",
    "new_zone_from_arrays",
    "new_zone_from_dict",
    "add",
    "merge",
    # IO functions
    "read",
    "read_from_path",
    "write_numpy",
    "read_numpy",
    # CGNS conversion
    "pyCGNSToNode",
    "nodeToPyCGNS",
]

from .core import (
    registerDefaultFactory,
    factory,
    Node,
    Array,
    new_node,
    pyCGNSToNode,
    nodeToPyCGNS,
)
from .core import io as _core_io
from .core.io import read
from .cgns import Base, Tree, Zone, add, merge, new_base, new_tree, new_zone_from_arrays, new_zone_from_dict

try:
    from .core.io import write_numpy, read_numpy
except ImportError:
    __all__.remove("write_numpy")
    __all__.remove("read_numpy")

registerDefaultFactory()


def zeros(*args, **kwargs):
    kwargs.setdefault("dtype", "double")
    builder = getattr(factory, f"zeros_{kwargs['dtype']}")
    kwargs.pop("dtype")
    return builder(*args, **kwargs)


factory.zeros = zeros


def read_from_path(filename, path, order="F", safe_mode=False, strict_types=True):
    """Read and return one node from a file.

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
    filename = os.fspath(filename)
    suffix = Path(filename).suffix.lower()
    lazy_reader = getattr(_core_io, "LazyHdf5Reader", None)

    if suffix in {".yaml", ".yml"} or lazy_reader is None:
        selected = read(filename, order=order, safe_mode=safe_mode, strict_types=strict_types).get_at_path(
            path, path_is_relative=True
        )
    else:
        reader = lazy_reader(filename, order=order, safe_mode=safe_mode, strict_types=strict_types)
        selected = reader.node_at_path(path)

    if selected is not None:
        selected.detach()
    return selected

# required for codes using noder as a dependency through cmake 
def get_package_dir() -> str:
    return str(Path(__file__).resolve().parent)


def get_include_dir() -> str:
    return str(Path(get_package_dir()) / "include")


def get_cmake_dir() -> str:
    return str(Path(get_package_dir()) / "cmake")

# obsolete if using bundling:
# def find_hdf5_dll_dir():
#     for path in os.environ["PATH"].split(";"):
#         path = path.strip()
#         if os.path.isdir(path) and glob.glob(os.path.join(path, "hdf5*.dll")):
#             return path 

# if sys.platform == "win32":
#     os.add_dll_directory(os.path.dirname(__file__))

#     hdf5_lib_dir = find_hdf5_dll_dir()
#     if hdf5_lib_dir: os.add_dll_directory(hdf5_lib_dir)
