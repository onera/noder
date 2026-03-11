# include "data/data_pybind.hpp"
# include "array/array_pybind.hpp"
# include "data/data_factory.hpp"
# include "node/node_pybind.hpp"
# include "node/navigation_pybind.hpp"
# include "io/cgns/node_pycgns_converter_pybind.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/io.hpp"
#endif


PYBIND11_MODULE(core, m) {

    m.def(
        "registerDefaultFactory",
        &registerDefaultFactory,
        R"doc(
Register the default data factory used by new nodes.

Returns
-------
None
)doc");

    #ifdef ENABLE_HDF5_IO
    // TODO redesign io pybindings
    py::module_ io_m = m.def_submodule(
        "io",
        R"doc(
HDF5/CGNS input-output helpers.

See C++ counterpart: :ref:`cpp-io-module`.
)doc");
    io_m.def(
        "read",
        &io::read,
        R"doc(
Read a CGNS-like HDF5 file into a Node hierarchy.

Parameters
----------
filename : str
    Input file path.

Returns
-------
Node
    Root node read from disk.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_io.py
   :language: python
   :pyobject: test_read
)doc",
        py::arg("filename"));
    io_m.def(
        "write_numpy",
        &io::write_numpy,
        R"doc(
Write a NumPy array to a dataset in an HDF5 file.

Parameters
----------
array : numpy.ndarray
    Array to persist.
filename : str
    Output file path.
dataset_name : str, optional
    Dataset name inside file.

Returns
-------
None

Example
-------
.. literalinclude:: ../../../tests/python/io/test_io.py
   :language: python
   :pyobject: test_write_and_read_numerical_numpy
)doc",
        py::arg("array"),
        py::arg("filename"),
        py::arg("dataset_name")=std::string("numpy"));
    io_m.def(
        "read_numpy",
        &io::read_numpy,
        R"doc(
Read a NumPy array from an HDF5 dataset.

Parameters
----------
filename : str
    Input file path.
dataset_name : str, optional
    Dataset name inside file.
order : str, optional
    Memory order of returned array (``"C"`` or ``"F"``).

Returns
-------
numpy.ndarray
    Loaded array.

Example
-------
.. literalinclude:: ../../../tests/python/io/test_io.py
   :language: python
   :pyobject: test_write_and_read_numerical_numpy
)doc",
        py::arg("filename"),
        py::arg("dataset_name")=std::string("numpy"),
        py::arg("order")=std::string("F"));
    #endif

    bindData(m);
    bindArray(m);
    bindNode(m);
    bindNavigation(m);
    bindNodePyCGNSConverter(m);

}

