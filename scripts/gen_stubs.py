#!/usr/bin/env python
"""Generate package stubs from a configured set of native modules.

The same script is used by the development install and by CMake's package
install step. Import roots and the output directory are explicit so a wheel
build always inspects and writes to the package being assembled.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Any, Callable


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CONFIG = ROOT / "scripts" / "stubgen_config.json"
_POSTPROCESSORS: dict[str, Callable[[str], str]] = {}


def load_config(path: Path) -> dict[str, Any]:
    config = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(config.get("package"), str) or not config["package"]:
        raise ValueError(f"{path} must define a non-empty package name")
    if not isinstance(config.get("modules"), list) or not config["modules"]:
        raise ValueError(f"{path} must define at least one native module")
    return config


def build_stubgen_environment(import_roots: list[Path]) -> dict[str, str]:
    env = os.environ.copy()
    paths = [str(path.resolve()) for path in import_roots]
    inherited = env.get("PYTHONPATH")
    if inherited:
        paths.append(inherited)
    env["PYTHONPATH"] = os.pathsep.join(paths)
    return env


def load_postprocessor(specification: str | None) -> Callable[[str], str] | None:
    if specification is None:
        return None
    if specification in _POSTPROCESSORS:
        return _POSTPROCESSORS[specification]

    module_name, separator, function_name = specification.partition(":")
    if not separator or not re.fullmatch(r"[A-Za-z_]\w*", module_name):
        raise ValueError(
            "postprocess must have the form 'module_name:function_name'"
        )
    module_path = ROOT / "scripts" / f"{module_name}.py"
    if not module_path.is_file():
        raise FileNotFoundError(f"Postprocessor module does not exist: {module_path}")

    module_spec = importlib.util.spec_from_file_location(
        f"_stubgen_postprocess_{module_name}", module_path
    )
    if module_spec is None or module_spec.loader is None:
        raise ImportError(f"Could not load postprocessor module: {module_path}")
    module = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(module)
    processor = getattr(module, function_name)
    if not callable(processor):
        raise TypeError(f"Configured postprocessor is not callable: {specification}")
    _POSTPROCESSORS[specification] = processor
    return processor


def find_stub_file(output_root: Path, package: str, module_name: str) -> Path:
    module_parts = module_name.split(".")
    if not module_parts or module_parts[0] != package:
        raise ValueError(
            f"Native module {module_name!r} must be inside package {package!r}"
        )
    module_path = output_root / package / Path(*module_parts[1:])
    candidates = (module_path / "__init__.pyi", module_path.with_suffix(".pyi"))
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError(
        "pybind11-stubgen did not create an expected stub; checked: "
        + ", ".join(str(candidate) for candidate in candidates)
    )


def generate_stubs(
    config: dict[str, Any], import_roots: list[Path], target_dir: Path
) -> list[Path]:
    package = config["package"]
    env = build_stubgen_environment(import_roots)
    generated: list[tuple[str, str]] = []

    with tempfile.TemporaryDirectory(prefix=f"{package}-stubgen-") as temp_dir:
        output_root = Path(temp_dir)
        for module in config["modules"]:
            module_name = module["name"]
            command = ["pybind11-stubgen", module_name, "-o", str(output_root)]
            for unresolved_name in module.get("ignore_unresolved_names", []):
                command.extend(("--ignore-unresolved-names", unresolved_name))
            subprocess.check_call(command, env=env)

            stub_path = find_stub_file(output_root, package, module_name)
            stub_text = stub_path.read_text(encoding="utf-8")
            processor = load_postprocessor(module.get("postprocess"))
            if processor is not None:
                stub_text = processor(stub_text)
            generated.append((module["filename"], stub_text))

    target_dir.mkdir(parents=True, exist_ok=True)
    written_paths = []
    for filename, content in generated:
        target_path = target_dir / filename
        target_path.write_text(content, encoding="utf-8")
        written_paths.append(target_path)

    package_stub = config.get("package_stub")
    if package_stub is not None:
        package_stub_path = target_dir / package_stub["filename"]
        if not package_stub_path.exists():
            package_stub_path.write_text(package_stub["content"], encoding="utf-8")
        written_paths.append(package_stub_path)

    return written_paths


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        type=Path,
        default=DEFAULT_CONFIG,
        help="JSON module configuration (defaults to scripts/stubgen_config.json)",
    )
    parser.add_argument(
        "--import-root",
        type=Path,
        action="append",
        help="Path prepended to PYTHONPATH; may be supplied more than once",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        help="Package directory to receive generated stubs (defaults to dist/dev/<package>)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    config = load_config(args.config.resolve())
    import_roots = args.import_root or [ROOT / "dist" / "dev"]
    target_dir = args.output_dir or ROOT / "dist" / "dev" / config["package"]
    written_paths = generate_stubs(config, import_roots, target_dir)
    for path in written_paths:
        print(f"[gen_stubs] Updated: {path}")


if __name__ == "__main__":
    main()
