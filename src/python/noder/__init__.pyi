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
from .io import read, write_numpy, read_numpy

__all__: list[str] = [
    "__version__",
    "Node",
    "Array",
    "factory",
    "registerDefaultFactory",
    "new_node",
    "zeros",
    "read",
    "write_numpy",
    "read_numpy",
    "pyCGNSToNode",
    "nodeToPyCGNS",
]

def zeros(*args, **kwargs: typing.Any) -> Array: ...

