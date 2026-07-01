"""
CGNS-focused Python entry points.
"""

from __future__ import annotations

import collections.abc

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


def _flatten_nodes(value):
    if isinstance(value, Node):
        yield value
        return

    if isinstance(value, (list, tuple)):
        for item in value:
            yield from _flatten_nodes(item)
        return

    raise TypeError(f"expected Node or nested list/tuple of Node, got {type(value).__name__}")


def _add_named_base(tree: Tree, base_name: str, value) -> None:
    base = Base(base_name)
    for node in _flatten_nodes(value):
        if isinstance(node, Tree):
            for zone in node.zones():
                base.add_child(zone, override_sibling_by_name=False)
        elif isinstance(node, Base):
            for zone in node.zones():
                base.add_child(zone, override_sibling_by_name=False)
        else:
            base.add_child(node, override_sibling_by_name=False)

    base.update_dimensions_from_zones()
    tree.add(base)


def add(*data1, **data2) -> Tree:
    """Collect nodes into a single CGNS Tree, following treelab-style rules."""
    tree = Tree()

    for item in data1:
        if isinstance(item, collections.abc.Mapping):
            for base_name, value in item.items():
                _add_named_base(tree, str(base_name), value)
        else:
            for node in _flatten_nodes(item):
                tree.add(node)

    for base_name, value in data2.items():
        _add_named_base(tree, base_name, value)

    tree.set_unique_base_names()
    return tree


def merge(nodes) -> Node:
    """Merge a non-empty sequence of nodes into a copied first node."""
    if not isinstance(nodes, (list, tuple)):
        raise TypeError("merge: nodes must be a list or tuple")
    if not nodes:
        raise ValueError("merge: nodes cannot be empty")
    if not all(isinstance(node, Node) for node in nodes):
        raise TypeError("merge: all items must be Node instances")

    merged_node = nodes[0].copy()
    for node in nodes[1:]:
        merged_node.merge(node)
    return merged_node


def new_zone_from_dict(name: str, arrays_by_name: collections.abc.Mapping) -> Zone:
    """Construct a structured Zone from an ordered mapping of names to arrays."""
    if not isinstance(arrays_by_name, collections.abc.Mapping):
        raise TypeError("new_zone_from_dict: arrays_by_name must be a mapping")
    return new_zone_from_arrays(name, list(arrays_by_name.keys()), list(arrays_by_name.values()))
