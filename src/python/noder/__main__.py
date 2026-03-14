import argparse
import sys

from . import get_cmake_dir, get_include_dir, get_package_dir


def main() -> int:
    parser = argparse.ArgumentParser(description="noder packaging helper")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--cmake-dir", action="store_true", help="Print the installed CMake package directory.")
    group.add_argument("--include-dir", action="store_true", help="Print the installed C++ headers directory.")
    group.add_argument("--package-dir", action="store_true", help="Print the installed noder package directory.")
    args = parser.parse_args()

    if args.cmake_dir:
        print(get_cmake_dir())
    elif args.include_dir:
        print(get_include_dir())
    elif args.package_dir:
        print(get_package_dir())
    else:
        parser.print_help(sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
