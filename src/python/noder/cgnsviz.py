"""Python environment launcher for the native cgnsviz application."""

import os
from pathlib import Path
import subprocess
import sys


def _native_executable() -> Path:
    package_directory = Path(__file__).resolve().parent
    executable_name = "cgnsviz.exe" if os.name == "nt" else "cgnsviz"
    executable = package_directory / executable_name
    if not executable.is_file():
        raise FileNotFoundError(
            "The cgnsviz native executable is not installed in "
            f"'{package_directory}'. Reinstall noder with CGNSVIZ enabled."
        )
    return executable


def main() -> int:
    try:
        executable = _native_executable()
    except FileNotFoundError as error:
        print(f"cgnsviz: {error}", file=sys.stderr)
        return 1

    environment = os.environ.copy()
    environment["PATH"] = os.pathsep.join(
        [str(executable.parent), environment.get("PATH", "")]
    )
    arguments = [str(executable), *sys.argv[1:]]
    try:
        return subprocess.run(arguments, env=environment, check=False).returncode
    except OSError as error:
        print(f"cgnsviz: cannot launch native executable: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
