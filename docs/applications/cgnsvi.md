# cgnsvi

`cgnsvi` is Noder's interactive terminal browser for CGNS/HDF5 files. It lets
you move through a tree, inspect node payloads, search for matching nodes and
open linked nodes lazily. Press `h` in the interface to show the key bindings
available in the current view.

## Open a file

```console
cgnsvi wing.cgns
```

Use `--help` to see all options. The most useful options are:

| Option | Purpose |
| --- | --- |
| `--safe-mode` or `-s` | Replace malformed nodes with corruption markers and continue where possible. |
| `--strict-types` | Use CGNS-compatible payload widths; this is the default. |
| `--no-strict-types` | Preserve supported nonstandard widths when reading. |
| `--order C` or `--order F` | Choose the memory order for array payloads; CGNS convention is `F`. |
| `--page-size N` | Set the number of children shown per page. |
| `--payload-limit N` | Limit payload values printed in the terminal. |
| `--max-chars N` | Limit the characters printed from a string payload. |
| `--non-interactive` | Print a file summary and exit, useful in scripts. |

For example, inspect a file safely and print a concise summary:

```console
cgnsvi --safe-mode --non-interactive wing.cgns
```

## Navigate and search

The tree view shows the hierarchy and a short payload preview. Use the arrow
keys to move through nodes. Press `h` for view-specific help. Search with `/`
to find nodes using Noder's predicate syntax; press `i` to search for nodes by
their stored CGNS/HDF5 attribute type, such as `B1` or `I4`. Matches appear in
the Match view, where you can select a result and inspect its path and payload.

When a node is an HDF5 link, cgnsvi marks it with its target path and file.
Entering the link loads its contents lazily, so the rest of the file need not
be loaded first.

## Useful workflows

Summarize a file without entering the interactive browser:

```console
cgnsvi --non-interactive simulation.cgns
```

Preserve narrow or otherwise nonstandard types for debugging:

```console
cgnsvi --no-strict-types simulation.cgns
```

Use the `i` inspector to locate all nodes stored with a particular HDF5 type.
For example, searching `B1` is useful when tracking boolean-like data in a
file written by a tool that does not normalize its types.
