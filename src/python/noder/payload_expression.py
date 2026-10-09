"""Restricted NumPy expressions for creating or replacing node payloads."""

from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import sys
from typing import Callable

import numpy as np


class PayloadExpressionError(ValueError):
    """Raised when a payload expression is invalid or unsafe."""


def _reference_parts(value: str) -> tuple[str, str] | None:
    """Parse both the compact and documented brace forms of a reference."""
    if value.startswith("{{") and value.endswith("}}"):
        inner = value[2:-2]
        if "}@{" in inner:
            filename, path = inner.split("}@{", 1)
            return filename, path
    if value.startswith("{") and value.endswith("}"):
        inner = value[1:-1]
        if "@" in inner:
            filename, path = inner.split("@", 1)
            return filename, path
        if inner:
            return "", inner
    return None


@dataclass
class _ReferenceTransformer(ast.NodeTransformer):
    resolver: Callable[[str, str], np.ndarray]
    environment: dict[str, object]

    def visit_Constant(self, node):
        if isinstance(node.value, str):
            parts = _reference_parts(node.value)
            if parts is not None:
                filename, path = parts
                try:
                    value = self.resolver(filename, path)
                except Exception as error:
                    raise PayloadExpressionError(str(error)) from error
                name = f"__payload_reference_{len(self.environment)}"
                self.environment[name] = value
                return ast.copy_location(ast.Name(id=name, ctx=ast.Load()), node)
        return node


class _SafeExpressionValidator(ast.NodeVisitor):
    """Permit NumPy expressions, literals, operators, and array slicing only."""

    _allowed = {
        ast.Expression,
        ast.Constant,
        ast.Name,
        ast.Attribute,
        ast.Call,
        ast.keyword,
        ast.Subscript,
        ast.Index,
        ast.Slice,
        ast.Tuple,
        ast.List,
        ast.BinOp,
        ast.UnaryOp,
        ast.BoolOp,
        ast.Compare,
        ast.IfExp,
        ast.Load,
        ast.Add,
        ast.Sub,
        ast.Mult,
        ast.Div,
        ast.FloorDiv,
        ast.Mod,
        ast.Pow,
        ast.MatMult,
        ast.UAdd,
        ast.USub,
        ast.And,
        ast.Or,
        ast.Eq,
        ast.NotEq,
        ast.Lt,
        ast.LtE,
        ast.Gt,
        ast.GtE,
    }

    def __init__(self, names: set[str]):
        self.names = names

    def generic_visit(self, node):
        if type(node) not in self._allowed:
            raise PayloadExpressionError(
                f"Expression element {type(node).__name__} is not allowed"
            )
        super().generic_visit(node)

    def visit_Name(self, node):
        if node.id not in self.names:
            raise PayloadExpressionError(f"Unknown name {node.id!r}")

    def visit_Attribute(self, node):
        if (
            not isinstance(node.value, ast.Name)
            or node.value.id != "np"
            or node.attr.startswith("_")
        ):
            raise PayloadExpressionError("Only public np.* functions are allowed")
        self.generic_visit(node)

    def visit_Call(self, node):
        if not (
            isinstance(node.func, ast.Attribute)
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == "np"
            and not node.func.attr.startswith("_")
        ):
            raise PayloadExpressionError(
                "Only calls to public np.* functions are allowed"
            )
        self.generic_visit(node)


def evaluate_payload_expression(
    expression: str,
    resolver: Callable[[str, str], np.ndarray],
):
    """Evaluate a restricted NumPy expression after resolving payload references."""
    expression = expression.strip()
    if not expression:
        raise PayloadExpressionError("The new payload expression is empty")
    try:
        tree = ast.parse(expression, mode="eval")
    except SyntaxError as error:
        raise PayloadExpressionError(f"Invalid expression: {error}") from error

    environment: dict[str, object] = {"np": np}
    tree = _ReferenceTransformer(resolver, environment).visit(tree)
    ast.fix_missing_locations(tree)
    _SafeExpressionValidator(set(environment)).visit(tree)
    try:
        return eval(
            compile(tree, "<noder payload expression>", "eval"),
            {"__builtins__": {}},
            environment,
        )
    except Exception as error:
        raise PayloadExpressionError(f"Could not evaluate expression: {error}") from error


def _find_node(root, path: str):
    node = root
    parts = [part for part in path.strip("/").split("/") if part]
    if parts and parts[0] == root.name():
        parts = parts[1:]
    for part in parts:
        node = node.pick().child_by_name(part)
        if node is None:
            raise PayloadExpressionError(f"Node path {path!r} was not found")
    return node


def _save_cgnsviz_result(expression: str, filename: str, node_path: str,
                         output_path: str, order: str) -> None:
    from noder.core.io import LazyHdf5Reader

    readers = {}

    def open_reader(requested_filename: str):
        resolved = str(Path(requested_filename).resolve())
        if resolved not in readers:
            readers[resolved] = LazyHdf5Reader(resolved, order=order)
        return readers[resolved]

    current_file = str(Path(filename).resolve())
    current_reader = open_reader(current_file)
    selected = _find_node(current_reader.root(), node_path)

    def resolve_reference(reference_filename: str, path: str):
        if not reference_filename:
            parent = selected.parent()
            if parent is None:
                raise PayloadExpressionError(
                    f"Sibling payload reference {path!r} requires a selected node"
                )
            node = parent.pick().child_by_name(path.strip("/"))
            if node is None:
                raise PayloadExpressionError(
                    f"Sibling node {path.strip('/')!r} was not found"
                )
        else:
            requested = Path(reference_filename)
            if requested.name == Path(current_file).name and not requested.is_absolute():
                reader = current_reader
            else:
                if not requested.is_absolute():
                    requested = Path.cwd() / requested
                reader = open_reader(str(requested))
            node = _find_node(reader.root(), path)

        if not node.has_data():
            raise PayloadExpressionError(f"Node {node.path()} has no payload")
        values = node.numpy()
        if values is None:
            raise PayloadExpressionError(
                f"Node {node.path()} has no NumPy-compatible payload"
            )
        result = np.array(values, copy=True)
        result.setflags(write=False)
        return result

    try:
        value = evaluate_payload_expression(expression, resolve_reference)
        output = Path(output_path)
        if value is None:
            output.write_bytes(b"NODER_NONE\n")
            return
        array = np.asarray(value)
        if array.dtype.kind not in "biufSU":
            raise PayloadExpressionError(
                f"Payload dtype {array.dtype} is not supported by CGNS/HDF5"
            )
        if array.dtype.kind in "iuf" and array.dtype.itemsize not in (1, 2, 4, 8):
            raise PayloadExpressionError(
                f"Payload dtype {array.dtype} is not supported by CGNS/HDF5"
            )
        if array.dtype.kind == "f" and array.dtype.itemsize not in (4, 8):
            raise PayloadExpressionError(
                f"Payload dtype {array.dtype} is not supported by CGNS/HDF5"
            )
        array = np.array(array, copy=True, order="C")
        if array.dtype.byteorder not in ("|", "="):
            array = array.astype(array.dtype.newbyteorder("="), copy=False)
        with output.open("wb") as stream:
            np.save(stream, array, allow_pickle=False)
    finally:
        for reader in readers.values():
            reader.close()


def _main(argv=None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cgnsviz-evaluate", nargs=5,
                        metavar=("EXPRESSION", "FILE", "NODE_PATH", "OUTPUT", "ORDER"))
    arguments = parser.parse_args(argv)
    if not arguments.cgnsviz_evaluate:
        parser.error("--cgnsviz-evaluate is required")
    expression_path, filename, node_path, output_path, order = arguments.cgnsviz_evaluate
    try:
        expression = Path(expression_path).read_text(encoding="utf-8")
        _save_cgnsviz_result(expression, filename, node_path, output_path, order)
    except Exception as error:
        print(f"Payload edit failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(_main())
