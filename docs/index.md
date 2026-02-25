# noder

`noder` (Node-Oriented Data Extraction & Representation) is a hybrid C++/Python library for hierarchical scientific data handling with CGNS-like node trees and NumPy-backed payloads.

## Core features

- Hierarchical `Node` graph with deterministic child ordering.
- Typed payload support through `Data` / `Array`.
- Fast navigation and tree querying utilities.
- CGNS-style link metadata support.
- HDF5 read/write support (when enabled at build time).
- Python-friendly APIs, including parameter-container conversion:
  - `set_parameters(...)`
  - `get_parameters(...)`

## Documentation map

```{toctree}
:maxdepth: 2

api/index
```

## Indices and tables

- {ref}`genindex`
- {ref}`modindex`
- {ref}`search`
