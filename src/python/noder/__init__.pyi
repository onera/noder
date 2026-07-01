# needs to mimik here what is declared in __init__.py

from __future__ import annotations
import typing

from .core import (
    registerDefaultFactory,
    factory,
    Node,
    Array,
    new_node,
    pyCGNSToNode,
    nodeToPyCGNS,
)
from .cgns import Base, Tree, Zone, add, merge, new_base, new_zone_from_arrays, new_zone_from_dict
from .io import read, write_numpy, read_numpy

__all__: list[str] = [
    "__version__",
    "Node",
    "Array",
    "Tree",
    "Base",
    "Zone",
    "factory",
    "registerDefaultFactory",
    "new_node",
    "zeros",
    "new_base",
    "new_zone_from_arrays",
    "new_zone_from_dict",
    "add",
    "merge",
    "read",
    "write_numpy",
    "read_numpy",
    "pyCGNSToNode",
    "nodeToPyCGNS",
]

def zeros(*args, **kwargs: typing.Any) -> Array: ...
