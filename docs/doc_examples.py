"""Attach executable examples to their documented Python API targets."""

from __future__ import annotations

import ast
import os
import re
from collections import defaultdict
from fnmatch import fnmatchcase
from pathlib import Path
from typing import DefaultDict


DOCS_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = DOCS_DIR.parent
DOC_TESTS_DIR = PROJECT_ROOT / "tests" / "doc"
_START_MARKER = re.compile(r"^\s*#\s*docs:start\s+([\w-]+)\s*$", re.MULTILINE)
_END_MARKER = re.compile(r"^\s*#\s*docs:end\s+([\w-]+)\s*$", re.MULTILINE)


def _doc_example_targets(function: ast.FunctionDef | ast.AsyncFunctionDef) -> list[str]:
    targets = []
    for decorator in function.decorator_list:
        if not isinstance(decorator, ast.Call):
            continue
        marker = decorator.func
        if not (
            isinstance(marker, ast.Attribute)
            and marker.attr == "doc_example"
            and isinstance(marker.value, ast.Attribute)
            and marker.value.attr == "mark"
            and isinstance(marker.value.value, ast.Name)
            and marker.value.value.id == "pytest"
        ):
            continue

        target = next(
            (
                keyword.value.value
                for keyword in decorator.keywords
                if keyword.arg == "target" and isinstance(keyword.value, ast.Constant)
            ),
            None,
        )
        if target is None and decorator.args and isinstance(decorator.args[0], ast.Constant):
            target = decorator.args[0].value
        if isinstance(target, str):
            targets.append(target)
    return targets


def _collect_examples() -> DefaultDict[str, list[tuple[Path, str]]]:
    examples: DefaultDict[str, list[tuple[Path, str]]] = defaultdict(list)
    if not DOC_TESTS_DIR.exists():
        return examples

    for filename in sorted(DOC_TESTS_DIR.rglob("*.py")):
        source = filename.read_text(encoding="utf-8")
        syntax = ast.parse(source, filename=str(filename))
        lines = source.splitlines()

        for function in syntax.body:
            if not isinstance(function, (ast.FunctionDef, ast.AsyncFunctionDef)):
                continue
            targets = _doc_example_targets(function)
            if not targets:
                continue

            function_end = function.end_lineno
            while function_end < len(lines) and (
                not lines[function_end].strip()
                or lines[function_end].startswith((" ", "\t"))
            ):
                function_end += 1
            function_source = "\n".join(lines[function.lineno - 1 : function_end])
            markers = [match.group(1) for match in _START_MARKER.finditer(function_source)]
            if not markers:
                raise ValueError(
                    f"Doc example {filename}:{function.lineno} declares a target "
                    "but has no '# docs:start' marker"
                )
            end_markers = {match.group(1) for match in _END_MARKER.finditer(function_source)}
            missing_end_markers = sorted(set(markers) - end_markers)
            if missing_end_markers:
                raise ValueError(
                    f"Doc example {filename}:{function.lineno} has no matching "
                    "'# docs:end' marker for: " + ", ".join(missing_end_markers)
                )

            for target in targets:
                for marker in markers:
                    entry = (filename, marker)
                    if entry not in examples[target]:
                        examples[target].append(entry)
    return examples


def _strip_stale_examples(lines: list[str]) -> list[str]:
    cleaned = []
    index = 0
    while index < len(lines):
        line = lines[index].strip()
        is_example_heading = line in {"Example", "Examples"}
        is_example_rubric = line in {
            ".. rubric:: Example",
            ".. rubric:: Examples",
            "Example:",
            "Examples:",
        }
        if (
            is_example_heading
            and index + 1 < len(lines)
            and re.fullmatch(r"[-=~^`:#*+]+", lines[index + 1].strip())
        ) or is_example_rubric:
            search_from = index + (2 if is_example_heading else 1)
            include_index = next(
                (
                    candidate
                    for candidate in range(search_from, len(lines))
                    if lines[candidate].lstrip().startswith(".. literalinclude::")
                ),
                None,
            )
            if include_index is not None:
                index = include_index + 1
                while index < len(lines) and (
                    not lines[index].strip() or lines[index].startswith(" ")
                ):
                    index += 1
                continue
            # Example prose and doctest blocks in compiled/stale docstrings
            # are replaced by the tested snippets registered above.
            index = len(lines)
            continue

        if lines[index].lstrip().startswith(".. literalinclude::"):
            index += 1
            while index < len(lines) and (not lines[index].strip() or lines[index].startswith(" ")):
                index += 1
            continue
        cleaned.append(lines[index])
        index += 1
    return cleaned


def _append_doc_examples(app, what, name, obj, options, lines) -> None:
    examples = _collect_examples()
    entries = [
        entry
        for target_pattern, target_entries in examples.items()
        if fnmatchcase(name, target_pattern)
        for entry in target_entries
    ]
    if not entries:
        return

    lines[:] = _strip_stale_examples(lines)
    if not any(line.strip() in {"Example", "Examples"} for line in lines):
        lines.extend(
            ["", ".. rst-class:: noder-api-examples", "", "Examples", "--------"]
        )

    document_dir = DOCS_DIR / Path(app.env.docname).parent
    for filename, marker in entries:
        relative_path = os.path.relpath(filename, document_dir).replace(os.sep, "/")
        lines.extend(
            [
                "",
                f".. literalinclude:: {relative_path}",
                "   :language: python",
                f"   :start-after: # docs:start {marker}",
                f"   :end-before: # docs:end {marker}",
                "   :dedent: 4",
            ]
        )


def setup(app):
    app.connect("autodoc-process-docstring", _append_doc_examples)
    return {"parallel_read_safe": True}
