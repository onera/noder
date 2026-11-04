# Build warnings

## macOS Homebrew HDF5 deployment target

The macOS wheel job currently links against Homebrew's `libhdf5.320.dylib` from
`macos-latest`. Its minimum OS version can be newer than the project's configured
`MACOSX_DEPLOYMENT_TARGET=14.0`, producing an `ld` warning. This warning comes
from the prebuilt Homebrew dependency, not Noder source.

The wheel repair script inspects bundled dylibs and raises the wheel's deployment
target to the newest minimum version it finds. This avoids advertising a wheel as
compatible with macOS versions that its HDF5 dependency cannot run on. Building
HDF5 from source with a lower deployment target could avoid the warning while
retaining macOS 14 support, but adds a substantial dependency build to CI. The
current workflow keeps the accurate deployment target and accepts this external
linker warning.
