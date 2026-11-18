# Tutorial

This tutorial introduces Noder's node model, typed NumPy payloads, search
shortcuts, CGNS constructors, file I/O and interoperability with CGNS/Python
tools. Each code block is included from
[`tests/doc/test_tutorial.py`](../tests/doc/test_tutorial.py) and runs in
the regular pytest suite.

## Imports

The examples use Noder's factory functions and CGNS helpers:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start imports
:end-before: docs:end imports
:caption: Imports used by the examples
```

## Build a general node tree

Use `new_node` to create nodes and attach them to their parent in one call.
This keeps tree construction concise while retaining direct access to each
node object:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start tree
:end-before: docs:end tree
:dedent: 4
:linenos:
:caption: Create nodes and attach them to a parent
```

## Keep payload types explicit

Nodes can hold integer, floating-point, boolean and complex NumPy arrays. The
payload keeps its NumPy dtype in memory:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start payload-types
:end-before: docs:end payload-types
:dedent: 4
:linenos:
:caption: Attach typed payloads with new_node
```

For CGNS/HDF5 interoperability, strict type handling is enabled by default.
When writing a CGNS file, narrow integer and boolean arrays are promoted to
CGNS-compatible storage types.

## Construct CGNS structures

For CGNS trees, use the dedicated helpers: `new_zone_from_arrays` or
`new_zone_from_dict` to create a structured zone, `new_base` to assemble zones
into a base, and `new_tree` to create a tree from named bases and zones.

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start cgns-constructors
:end-before: docs:end cgns-constructors
:dedent: 4
:linenos:
:caption: Create zones, a base and a tree
```

## Search nodes

Use exact or glob searches for simple lookups. Predicates combine name and
type tests in one expression:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start search
:end-before: docs:end search
:dedent: 4
:linenos:
:caption: Search with pick, get and group
```

```{hint}
`node.get(...)` is a shortcut for `node.pick().by_and_glob(...)`, returning
the first match. `node.group(...)` is a shortcut for
`node.pick().all_by_and_glob(...)`, returning every match. Both accept glob
conditions for `name`, `type` and string `data`.
```

`by_*` methods return the first match and `all_by_*` methods return all
matches. Glob patterns support `*` and `?`; predicate expressions support
selectors such as `n:` for name and `t:` for node type.

## Save and read a YAML tree

YAML is useful for small files that should remain easy to inspect by hand. This
round-trip writes a general node tree and loads it again:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start yaml-round-trip
:end-before: docs:end yaml-round-trip
:dedent: 4
:linenos:
:caption: Write and read YAML
```

## Save and read a CGNS/HDF5 file

When Noder is built with HDF5 support, `new_tree` can construct the CGNS tree
that `tree.write()` saves to a `.cgns` file. `noder.read()` loads it again:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start cgns-round-trip
:end-before: docs:end cgns-round-trip
:dedent: 4
:linenos:
:caption: Write and read CGNS/HDF5
```

The example uses a structured `Zone_t` with coordinates and a `Pressure`
field. HDF5 reading can also be lazy, allowing large trees to be explored
before loading every payload.

## Exchange with CGNS/Python tools

Maia and Cassiopee use CGNS/Python tree values. Noder converts to and from that
representation while retaining the tree structure:

```{literalinclude} ../tests/doc/test_tutorial.py
:language: python
:start-after: docs:start cgns-python
:end-before: docs:end cgns-python
:dedent: 4
:linenos:
:caption: Convert between Noder and CGNS/Python trees
```

`nodeToPyCGNS()` returns the conventional `[name, value, children, type]`
structure; `pyCGNSToNode()` converts it back. Install Maia or Cassiopee
separately in the same Python environment when you want to pass the converted
tree to those tools.
