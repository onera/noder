# Noder

**NODER** (Node-Oriented Data Extraction & Representation) is a cross-platform
C++ and Python library for working with hierarchical scientific data.

Noder represents a dataset as a tree of objects. Each `Node` carries its name,
type, payload and child nodes together, so code can navigate and edit the data
through the objects that represent it. Numerical payloads are backed by NumPy
arrays and can keep their original scalar type in memory.

## Main features

- An object-oriented node tree with deterministic child ordering, parent
  relationships and CGNS-style links.
- Typed numerical payloads exposed through NumPy, including integer, floating
  point, boolean and complex arrays.
- Search by name, type, data, glob patterns or combined predicates.
- CGNS/HDF5 and YAML read/write support, including lazy access to HDF5 trees
  when built with HDF5 support.
- Conversion to and from the CGNS/Python tree representation used by tools such
  as Maia and Cassiopee.
- The `cgnsvi` terminal application for browsing, searching and inspecting
  CGNS/HDF5 files.

The Python interface is the main user-facing API. The native C++ core provides
the implementation used by Python, and Noder is built for Linux, Windows and
macOS. Published Python distributions let users install the library with pip
without configuring a source build on supported platforms.
