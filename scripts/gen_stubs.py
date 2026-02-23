#!/usr/bin/env python
from __future__ import annotations

import importlib
import inspect
import os
import re
import shutil
import subprocess
from pathlib import Path
from typing import Dict


# Adjust if the package name changes
PACKAGE_NAME = "noder"
MODULE_NAME = f"{PACKAGE_NAME}.core"

# Repository layout assumptions:
#   <ROOT>/
#       scripts/gen_stubs.py   (this file)
#       src/python/noder/...
#       dist/dev/              (cmake --install --prefix dist/dev)
ROOT = Path(__file__).resolve().parent.parent
SRC_PKG_DIR = ROOT / "src" / "python" / PACKAGE_NAME
DIST_DEV = ROOT / "dist" / "dev"


def is_dev_checkout() -> bool:
    """
    Heuristic: we are in a git/development checkout if src/python/noder/__init__.py exists.
    """
    return (SRC_PKG_DIR / "__init__.py").exists()


def find_installed_package_dir() -> Path:
    """
    Locate the directory where 'noder' is installed (site-packages) when running
    in a pip-installed context (outside the source tree).
    """
    pkg = importlib.import_module(PACKAGE_NAME)
    return Path(inspect.getfile(pkg)).resolve().parent


def build_stubgen_env() -> Dict[str, str]:
    """
    Build the environment for pybind11-stubgen.

    In dev mode, ensure that dist/dev is on PYTHONPATH so that 'import noder.core'
    resolves to the freshly built extension installed by CMake.
    """
    env = os.environ.copy()
    if is_dev_checkout() and DIST_DEV.exists():
        env["PYTHONPATH"] = f"{DIST_DEV}{os.pathsep}{env.get('PYTHONPATH', '')}"
    return env


def choose_target_dir() -> Path:
    """
    Decide where to write the final .pyi files.

    - In dev checkout: src/python/noder/
    - In pip-installed environment: site-packages/noder/
    """
    if is_dev_checkout():
        return SRC_PKG_DIR
    return find_installed_package_dir()


def ensure_typing_import(stub_text: str) -> str:
    """
    Ensure 'import typing' exists so inserted @typing.overload decorators resolve.
    """
    if re.search(r"^import typing\s*$", stub_text, flags=re.MULTILINE):
        return stub_text

    lines = stub_text.splitlines(keepends=True)
    insert_at = 0

    # Keep __future__ imports at the very top.
    while insert_at < len(lines) and lines[insert_at].startswith("from __future__ import "):
        insert_at += 1

    lines.insert(insert_at, "import typing\n")
    return "".join(lines)


def add_overload_decorators_for_repeated_method(
    stub_text: str, class_name: str, method_name: str
) -> str:
    """
    Some pybind11-stubgen outputs repeated methods without @typing.overload.
    Pylance keeps only the last signature and reports false operator errors.
    This adds @typing.overload above repeated method definitions in one class.
    """
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

    method_indices = [i for i in range(class_idx + 1, end_idx) if lines[i].startswith(method_prefix)]
    if len(method_indices) < 2:
        return stub_text

    offset = 0
    changed = False
    for idx in method_indices:
        idx += offset
        prev = lines[idx - 1].strip() if idx - 1 >= 0 else ""
        if prev != "@typing.overload":
            lines.insert(idx, "    @typing.overload\n")
            offset += 1
            changed = True

    if not changed:
        return stub_text

    return "".join(lines)


def postprocess_core_stub(stub_text: str) -> str:
    """
    Apply deterministic fixes on generated stubs for better IDE typing behavior.
    """
    patched = add_overload_decorators_for_repeated_method(
        stub_text, class_name="_NodeGroup", method_name="__add__"
    )
    if patched != stub_text:
        patched = ensure_typing_import(patched)
    return patched


def main() -> None:
    env = build_stubgen_env()
    out_dir = ROOT / "_stubgen_out"

    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # Run pybind11-stubgen on the compiled module.
    # --ignore-unresolved-names Node is needed because 'io.read' mentions Node.
    subprocess.check_call(
        [
            "pybind11-stubgen",
            MODULE_NAME,
            "-o",
            str(out_dir),
            "--ignore-unresolved-names",
            "Node",
        ],
        env=env,
    )

    # Expected structure:
    #   _stubgen_out/
    #       noder/
    #           core/
    #               __init__.pyi   <-- main stub for noder.core
    #               io.pyi         <-- optional submodule stub (not used for now)
    #               factory.pyi    <-- optional submodule stub (not used for now)
    generated_pkg_dir = out_dir / PACKAGE_NAME
    core_pkg_dir = generated_pkg_dir / "core"
    core_init_pyi = core_pkg_dir / "__init__.pyi"

    if not core_init_pyi.exists():
        available = list(core_pkg_dir.glob("*.pyi")) if core_pkg_dir.exists() else []
        raise SystemExit(
            f"Did not find expected stub at {core_init_pyi}.\n"
            f"Available in {core_pkg_dir}: {available}"
        )

    target_dir = choose_target_dir()
    target_dir.mkdir(parents=True, exist_ok=True)

    generated_core_text = core_init_pyi.read_text(encoding="utf-8")
    generated_core_text = postprocess_core_stub(generated_core_text)

    # Flatten noder/core/__init__.pyi -> noder/core.pyi
    target_core_pyi = target_dir / "core.pyi"
    # Overwrite existing stub (we want it to match current bindings + postprocess fixes)
    target_core_pyi.write_text(generated_core_text, encoding="utf-8")

    # In dev mode, optionally mirror the stub to dist/dev/noder/core.pyi when present.
    # This keeps IDE typing consistent if the interpreter points at dist/dev.
    if is_dev_checkout():
        dist_core_pyi = DIST_DEV / PACKAGE_NAME / "core.pyi"
        if dist_core_pyi.exists():
            dist_core_pyi.write_text(generated_core_text, encoding="utf-8")

    # We currently ignore submodule stubs (io.pyi, factory.pyi, ...) because
    # your public Python API is 'from noder.core import Node, Array, ...'.
    # If you later want to expose noder.core.io, we can extend this script
    # to merge those stubs as well.

    # Ensure noder/__init__.pyi exists (if not, create a minimal one).
    # We DO NOT overwrite an existing __init__.pyi to allow manual refinements.
    target_init_pyi = target_dir / "__init__.pyi"
    if not target_init_pyi.exists():
        # Adjust exports to mirror your real __init__.py public API.
        # For now, keep it minimal and you can edit it by hand later.
        target_init_pyi.write_text(
            "# Stub for noder.__init__\n"
            "# Adjust exports to match your public API as needed.\n"
            "from .core import Node, Array  # type: ignore[misc]\n",
            encoding="utf-8",
        )

    # Cleanup temporary directory
    shutil.rmtree(out_dir)

    print(f"[gen_stubs] Updated main stub: {target_core_pyi}")
    print(f"[gen_stubs] Ensured package stub: {target_init_pyi}")


if __name__ == "__main__":
    main()
