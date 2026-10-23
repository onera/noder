"""Noder-specific cleanup for pybind11-stubgen output."""
from __future__ import annotations

import re


def ensure_typing_import(stub_text: str) -> str:
    if re.search(r"^import typing\s*$", stub_text, flags=re.MULTILINE):
        return stub_text
    lines = stub_text.splitlines(keepends=True)
    insert_at = 0
    while insert_at < len(lines) and lines[insert_at].startswith("from __future__ import "):
        insert_at += 1
    lines.insert(insert_at, "import typing\n")
    return "".join(lines)


def ensure_numpy_import(stub_text: str) -> str:
    if re.search(r"^import numpy\s*$", stub_text, flags=re.MULTILINE):
        return stub_text
    lines = stub_text.splitlines(keepends=True)
    insert_at = 0
    while insert_at < len(lines) and lines[insert_at].startswith("from __future__ import "):
        insert_at += 1
    lines.insert(insert_at, "import numpy\n")
    return "".join(lines)


def add_overload_decorators_for_repeated_method(
    stub_text: str, class_name: str, method_name: str
) -> str:
    """Decorate repeated pybind overloads so IDEs retain every signature."""
    lines = stub_text.splitlines(keepends=True)
    class_header = f"class {class_name}:"
    method_prefix = f"    def {method_name}("
    class_idx = next((i for i, line in enumerate(lines) if line.startswith(class_header)), -1)
    if class_idx < 0:
        return stub_text

    end_idx = class_idx + 1
    while end_idx < len(lines):
        line = lines[end_idx]
        if line.strip() == "":
            end_idx += 1
            continue
        if not line.startswith(" "):
            break
        end_idx += 1

    method_indices = [
        i for i in range(class_idx + 1, end_idx) if lines[i].startswith(method_prefix)
    ]
    if len(method_indices) < 2:
        return stub_text

    offset = 0
    changed = False
    for idx in method_indices:
        idx += offset
        previous = lines[idx - 1].strip() if idx - 1 >= 0 else ""
        if previous != "@typing.overload":
            lines.insert(idx, "    @typing.overload\n")
            offset += 1
            changed = True
    if not changed:
        return stub_text
    return "".join(lines)


def postprocess_core_stub(stub_text: str) -> str:
    patched = add_overload_decorators_for_repeated_method(
        stub_text, class_name="_NodeGroup", method_name="__add__"
    )
    if patched != stub_text:
        patched = ensure_typing_import(patched)
    patched = patched.replace(
        "def getPyArray(self) -> typing.Any:",
        "def getPyArray(self) -> numpy.ndarray:",
    )
    if patched != stub_text:
        patched = ensure_numpy_import(patched)
    return patched
