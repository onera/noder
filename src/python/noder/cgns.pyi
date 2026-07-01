from __future__ import annotations

import collections.abc
import typing

from .core import Node
from ._cgns import Base, Tree, Zone, new_base, new_zone_from_arrays

__all__ = [
    "Base",
    "Tree",
    "Zone",
    "add",
    "merge",
    "new_base",
    "new_zone_from_arrays",
    "new_zone_from_dict",
]

def add(*data1: typing.Any, **data2: typing.Any) -> Tree: ...
def merge(nodes: collections.abc.Sequence[Node]) -> Node: ...
def new_zone_from_dict(name: str, arrays_by_name: collections.abc.Mapping[str, typing.Any]) -> Zone: ...
