"""Noder-specific cleanup for pybind11-stubgen output."""
from __future__ import annotations

import ast
import re


def _insert_module_import(stub_text: str, import_statement: str) -> str:
    lines = stub_text.splitlines(keepends=True)
    module = ast.parse(stub_text)
    insert_at = 0
    if module.body:
        first_statement = module.body[0]
        if (
            isinstance(first_statement, ast.Expr)
            and isinstance(first_statement.value, (ast.Constant, ast.Str))
            and isinstance(getattr(first_statement.value, "value", None), str)
        ):
            insert_at = first_statement.end_lineno or first_statement.lineno
    for statement in module.body:
        if isinstance(statement, ast.ImportFrom) and statement.module == "__future__":
            insert_at = max(insert_at, statement.end_lineno or statement.lineno)
    lines.insert(insert_at, import_statement + "\n")
    return "".join(lines)


def ensure_typing_import(stub_text: str) -> str:
    if re.search(r"^import typing\s*$", stub_text, flags=re.MULTILINE):
        return stub_text
    return _insert_module_import(stub_text, "import typing")


def ensure_numpy_import(stub_text: str) -> str:
    if re.search(r"^import numpy\s*$", stub_text, flags=re.MULTILINE):
        return stub_text
    return _insert_module_import(stub_text, "import numpy")


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


def postprocess_package_stub(stub_text: str) -> str:
    """Restore useful annotations for Python wrappers in the package stub."""
    patched = re.sub(
        r"^def read_from_path\(.*\):$",
        "def read_from_path(\n"
        "    filename: str | Path,\n"
        "    path: str,\n"
        "    order: str = \"F\",\n"
        "    safe_mode: bool = False,\n"
        "    strict_types: bool = True,\n"
        ") -> Node | None:",
        stub_text,
        count=1,
        flags=re.MULTILINE,
    )
    if patched == stub_text:
        raise ValueError("Generated package stub does not contain read_from_path")
    previous = patched
    patched = re.sub(
        r"^def zeros\(.*\):$",
        "def zeros(*args: typing.Any, **kwargs: typing.Any) -> Array:",
        patched,
        count=1,
        flags=re.MULTILINE,
    )
    if patched == previous:
        raise ValueError("Generated package stub does not contain zeros")

    module = ast.parse(patched)
    exported_names: set[str] | None = None
    declared_names: set[str] = set()
    for statement in module.body:
        if isinstance(statement, ast.ImportFrom):
            declared_names.update(alias.asname or alias.name for alias in statement.names)
        elif isinstance(statement, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            declared_names.add(statement.name)
        elif isinstance(statement, ast.AnnAssign) and isinstance(statement.target, ast.Name):
            declared_names.add(statement.target.id)
            if statement.target.id == "__all__":
                try:
                    exported_names = set(ast.literal_eval(statement.value))
                except (ValueError, TypeError):
                    pass
        elif isinstance(statement, ast.Assign):
            for target in statement.targets:
                if isinstance(target, ast.Name):
                    declared_names.add(target.id)
                    if target.id == "__all__":
                        try:
                            exported_names = set(ast.literal_eval(statement.value))
                        except (ValueError, TypeError):
                            pass

    if exported_names is None:
        raise ValueError("Generated package stub does not define a literal __all__")
    missing_names = sorted(exported_names - declared_names)
    if missing_names:
        raise ValueError(
            "Generated package stub is missing public exports: " + ", ".join(missing_names)
        )

    return ensure_typing_import(patched)
