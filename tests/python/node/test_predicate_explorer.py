import numpy as np
import pytest

from noder.core import Node


def _tree():
    root = Node("root", "Root_t")
    branch = Node("branch", "UserDefinedData_t")
    branch.attach_to(root)

    proc_a = Node("procA", "DataArray_t")
    proc_a.set_data(7)
    proc_a.attach_to(branch)

    proc_b = Node("procB", "DataArray_t")
    proc_b.set_data(np.array([8, 9], dtype=np.int32))
    proc_b.attach_to(branch)

    blade = Node("blade", "FamilyName_t")
    blade.set_data("BLADE")
    blade.attach_to(branch)

    scientific = Node("scientific", "DataArray_t")
    scientific.set_data(1e-3)
    scientific.attach_to(branch)

    literal = Node("left&right", "A|B")
    literal.set_data("A|B")
    literal.attach_to(root)

    deep = Node("deep", "DataArray_t")
    deep.set_data(12)
    deep.attach_to(proc_a)
    return root, branch, proc_a, proc_b, blade, literal, deep, scientific


def test_predicate_name_type_data_and_directional_levels():
    root, branch, proc_a, proc_b, blade, _, deep, scientific = _tree()

    matches = root.pick().all_by_predicate("/ n:proc* & t:DataArray_t & d:>5")
    assert matches == [proc_a]

    assert root.pick().by_predicate("/ n:proc*") is proc_a
    assert root.pick().by_predicate("//n:procA") is proc_a
    assert root.pick().all_by_predicate("/ l:1 & n:*") == [branch, root.pick().by_predicate('/n:"left&right"')]
    assert root.pick().all_by_predicate("/ l:3 & n:deep") == [deep]
    assert deep.pick().all_by_predicate(r"\ l:1 & t:DataArray_t") == [proc_a]
    assert deep.pick().all_by_predicate(r"\ l:<=2 & n:*") == [proc_a, branch]

    # A second traversal block starts from the matches of the first one.
    assert root.pick().all_by_predicate("/ n:branch / n:procB") == [proc_b]

    # Numeric predicates only match scalar-convertible payloads.
    assert root.pick().all_by_predicate("/ d:>6") == [proc_a, deep]
    assert root.pick().all_by_predicate("/ d:>=+7") == [proc_a, deep]
    assert root.pick().all_by_predicate("/ d:1e-3") == [scientific]


def test_predicate_boolean_precedence_and_quoted_operator_literals():
    root, _, _, _, blade, literal, _, _ = _tree()

    assert root.pick().all_by_predicate('/ (n:procA | n:blade) & t:*') == [
        root.pick().by_predicate('/n:procA'),
        blade,
    ]
    assert root.pick().all_by_predicate('/n:"left&right"') == [literal]
    assert root.pick().all_by_predicate('/t:"A|B" & d:"A|B"') == [literal]
    assert root.pick().all_by_predicate('/t:FamilyName_t & d:BLA*') == [blade]


def test_predicate_ancestor_deduplication_and_empty_results():
    root, branch, proc_a, _, _, _, _, _ = _tree()

    # Both selected descendants produce the same parent; it appears once.
    assert root.pick().all_by_predicate("/n:proc* \\n:branch") == [branch]
    assert root.pick().all_by_predicate("/n:does-not-exist") == []
    assert root.pick().all_by_predicate("/n:does-not-exist /n:*") == []
    assert proc_a.pick().all_by_predicate(r"\n:root") == [root]


@pytest.mark.parametrize(
    "expression",
    ["", "/", "/x:value", "/n:", "/n:a &", "/(n:a", "/l:abc", "/d:>abc"],
)
def test_predicate_rejects_malformed_expressions(expression):
    root, *_ = _tree()
    with pytest.raises(ValueError):
        root.pick().all_by_predicate(expression)


def test_predicate_explorer_works_with_lazy_cgns_nodes(tmp_path):
    h5py = pytest.importorskip("h5py")
    try:
        from noder.core import io as gio
    except ImportError:
        pytest.skip("HDF5 support is not enabled")
    if not hasattr(gio, "LazyHdf5Reader"):
        pytest.skip("HDF5 support is not enabled")

    filename = tmp_path / "predicate-lazy.cgns"
    with h5py.File(filename, "w", track_order=True) as h5file:
        h5file.attrs["name"] = np.bytes_("HDF5 MotherNode")
        h5file.attrs["label"] = np.bytes_("Root Node of HDF5 File")
        h5file.attrs["type"] = np.bytes_("MT")

        for name, cgns_type, payload in [
            ("Small", "I4", np.array([42], dtype=np.int32)),
            ("Large", "I4", np.arange(32, dtype=np.int32)),
        ]:
            group = h5file.create_group(name, track_order=True)
            group.attrs["name"] = np.bytes_(name)
            group.attrs["label"] = np.bytes_("DataArray_t")
            group.attrs["type"] = np.bytes_(cgns_type)
            group.create_dataset(" data", data=payload)

    reader = gio.LazyHdf5Reader(str(filename))
    root = reader.root()
    assert root.loaded_children() == []
    assert [node.name() for node in root.pick().all_by_predicate("/t:DataArray_t")] == [
        "Small", "Large"
    ]
    large = root.pick().by_predicate("/n:Large")
    assert large is not None
    assert large.data_is_scalar() is False
    assert root.pick().all_by_predicate("/d:>40") == [root.pick().by_predicate("/n:Small")]
    reader.close()
