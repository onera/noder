from __future__ import annotations

import ast
import importlib.util
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "gen_stubs.py"
SPEC = importlib.util.spec_from_file_location("noder_stubgen_test", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
stubgen = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(stubgen)


def test_configured_generation_writes_flat_stubs_and_postprocesses_core(
    tmp_path: Path, monkeypatch
) -> None:
    config = stubgen.load_config(ROOT / "scripts" / "stubgen_config.json")
    generated_core = (
        "class _NodeGroup:\n"
        "    def __add__(self, other: int) -> int: ...\n"
        "    def __add__(self, other: str) -> str: ...\n"
        "    def getPyArray(self) -> typing.Any: ...\n"
    )
    generated_package = (
        "__all__ = ['new_tree', 'read_from_path', 'zeros']\n"
        "from .cgns import new_tree\n"
        "from noder.core import Array, Node\n"
        "from pathlib import Path\n"
        "def read_from_path(filename, path, order = 'F', safe_mode = False, strict_types = True):\n"
        "    ...\n"
        "def zeros(*args, **kwargs):\n"
        "    ...\n"
    )

    def fake_stubgen(command: list[str], env: dict[str, str]) -> None:
        module_name = command[1]
        output_root = Path(command[command.index("-o") + 1])
        module_path = output_root / config["package"] / Path(*module_name.split(".")[1:])
        stub_path = module_path / "__init__.pyi"
        stub_path.parent.mkdir(parents=True, exist_ok=True)
        stub_path.write_text(
            generated_core
            if module_name == "noder.core"
            else generated_package
            if module_name == "noder"
            else "# generated\n",
            encoding="utf-8",
        )

    monkeypatch.setattr(stubgen.subprocess, "check_call", fake_stubgen)
    target_dir = tmp_path / "noder"
    target_dir.mkdir()
    (target_dir / "__init__.pyi").write_text("# stale stub\n", encoding="utf-8")
    source_package_dir = tmp_path / "source" / "noder"
    written = stubgen.generate_stubs(
        config,
        [tmp_path],
        target_dir,
        source_package_dir=source_package_dir,
    )

    assert {path.name for path in written} == {
        "core.pyi",
        "_cgns.pyi",
        "io.pyi",
        "__init__.pyi",
    }
    core_stub = (target_dir / "core.pyi").read_text(encoding="utf-8")
    assert "@typing.overload" in core_stub
    assert "import typing" in core_stub
    assert "import numpy" in core_stub
    assert "def getPyArray(self) -> numpy.ndarray:" in core_stub
    package_stub = (target_dir / "__init__.pyi").read_text(encoding="utf-8")
    ast.parse(package_stub)
    assert "new_tree" in package_stub
    assert "stale stub" not in package_stub
    assert "def read_from_path(" in package_stub
    assert "filename: str | Path" in package_stub
    assert "def zeros(*args: typing.Any, **kwargs: typing.Any) -> Array:" in package_stub
    assert "import typing" in package_stub
    assert all(line == line.rstrip() for line in package_stub.splitlines())
    assert (source_package_dir / "__init__.pyi").read_text(encoding="utf-8") == package_stub


def test_import_insertion_handles_module_docstring_and_future_import(monkeypatch) -> None:
    # Simulate Python 3.14, where ast.Str was removed, on older interpreters too.
    original_getattr = ast.__getattr__

    def without_legacy_str(name: str):
        if name == "Str":
            raise AttributeError(name)
        return original_getattr(name)

    monkeypatch.setattr(ast, "__getattr__", without_legacy_str)
    postprocess = stubgen.load_postprocessor(
        "stubgen_postprocess:ensure_typing_import"
    )
    assert postprocess is not None
    result = postprocess(
        '"""Generated module."""\n'
        "from __future__ import annotations\n"
        "value: str\n"
    )
    ast.parse(result)
    assert result.index('"""Generated module."""') < result.index(
        "from __future__ import annotations"
    ) < result.index("import typing") < result.index("value: str")
