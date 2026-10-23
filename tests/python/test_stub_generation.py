from __future__ import annotations

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

    def fake_stubgen(command: list[str], env: dict[str, str]) -> None:
        module_name = command[1]
        output_root = Path(command[command.index("-o") + 1])
        module_path = output_root / config["package"] / Path(*module_name.split(".")[1:])
        stub_path = module_path / "__init__.pyi"
        stub_path.parent.mkdir(parents=True, exist_ok=True)
        stub_path.write_text(
            generated_core if module_name == "noder.core" else "# generated\n",
            encoding="utf-8",
        )

    monkeypatch.setattr(stubgen.subprocess, "check_call", fake_stubgen)
    target_dir = tmp_path / "noder"
    written = stubgen.generate_stubs(config, [tmp_path], target_dir)

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
    assert (target_dir / "__init__.pyi").is_file()
