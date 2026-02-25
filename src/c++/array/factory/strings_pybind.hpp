# ifndef ARRAY_FACTORY_STRINGS_PYBIND_HPP
# define ARRAY_FACTORY_STRINGS_PYBIND_HPP

# include <pybind11/pybind11.h>
# include "array/factory/strings.hpp"

using namespace arrayfactory;

void bindFactoryOfArraysFromStrings(py::module_ &m) {

    m.def(
        "arrayFromString",
        &arrayFromString,
        py::arg("string"),
        R"doc(
Convert UTF-8 text into a string-backed :py:class:`noder.core.Array`.

Parameters
----------
string : str
    Input UTF-8 text.

Returns
-------
Array
    String payload array.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_strings.py
   :language: python
   :pyobject: test_arrayFromString
)doc");

    m.def(
        "arrayFromUnicodeString",
        &arrayFromUnicodeString,
        py::arg("string"),
        R"doc(
Convert text into a Unicode (``U`` dtype) :py:class:`noder.core.Array`.

Parameters
----------
string : str
    Input text.

Returns
-------
Array
    Unicode payload array.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_strings.py
   :language: python
   :pyobject: test_arrayFromUnicodeString
)doc");

}

# endif // ARRAY_FACTORY_STRINGS_PYBIND_HPP
