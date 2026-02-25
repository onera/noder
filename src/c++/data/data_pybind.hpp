# include "data/data.hpp"

# include <pybind11/numpy.h>
# include <pybind11/pybind11.h>

namespace py = pybind11;

void bindData(py::module_ &m) {
    py::class_<Data, std::shared_ptr<Data>>(
        m,
        "Data",
        R"doc(
Abstract payload interface used by :py:class:`noder.core.Node`.

This class is normally returned by :py:meth:`noder.core.Node.data` and most
concrete payloads are :py:class:`noder.core.Array`.

See C++ counterpart: :ref:`cpp-data-class`.
)doc")
    .def("hasString", &Data::hasString, R"doc(
Check whether this payload represents a string-like value.

Returns
-------
bool
    ``True`` when the payload stores string-compatible data.

Example
-------
.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :pyobject: test_dataInt
)doc")
    .def("isNone", &Data::isNone, R"doc(
Check whether this payload is empty (None-like).

Returns
-------
bool
    ``True`` when payload is considered empty.

Example
-------
.. literalinclude:: ../../../tests/python/node/test_node.py
   :language: python
   :pyobject: test_parent_empty
)doc")
    .def("isScalar", &Data::isScalar, R"doc(
Check whether this payload is a scalar numeric value.

Returns
-------
bool
    ``True`` when payload corresponds to a scalar.

Example
-------
.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :pyobject: test_isScalar
)doc")
    .def("extractString", &Data::extractString, R"doc(
Extract the payload as a Python string.

Returns
-------
str
    UTF-8 string representation of the payload.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_strings.py
   :language: python
   :pyobject: test_arrayFromString
)doc");
}
