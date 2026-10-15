"""Python environment launcher for the native cgnsvi application."""

import os
from pathlib import Path
import subprocess
import sys


def _native_executable() -> Path:
    package_directory = Path(__file__).resolve().parent
    executable_name = "cgnsvi.exe" if os.name == "nt" else "cgnsvi"
    executable = package_directory / executable_name
    if not executable.is_file():
        raise FileNotFoundError(
            "The cgnsvi native executable is not installed in "
            f"'{package_directory}'. Reinstall noder with CGNSVI enabled."
        )
    return executable


def main() -> int:
    try:
        executable = _native_executable()
    except FileNotFoundError as error:
        print(f"cgnsvi: {error}", file=sys.stderr)
        return 1

    environment = os.environ.copy()
    environment["PATH"] = os.pathsep.join(
        [str(executable.parent), environment.get("PATH", "")]
    )
    environment["NODER_PYTHON_EXECUTABLE"] = sys.executable
    arguments = [str(executable), *sys.argv[1:]]
    try:
        return subprocess.run(arguments, env=environment, check=False).returncode
    except OSError as error:
        print(f"cgnsvi: cannot launch native executable: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
