# include "array/array.hpp"
# include "array/factory/factory_pybind.hpp"

void bindArray(py::module_ &m) {

    py::class_<Array, Data, std::shared_ptr<Array>>(
        m,
        "Array",
        R"doc(
NumPy-backed concrete :py:class:`noder.core.Data` payload.

`Array` stores either numeric values or string-like values and exposes
lightweight helpers for shape, indexing, contiguity, and printing.

See C++ counterpart: :ref:`cpp-array-class`.
)doc")

        .def(py::init<>(), R"doc(
Construct an empty (None-like) array payload.

Example
-------
.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :pyobject: test_constructorEmpty
)doc")
        .def(py::init<const py::array&>(), R"doc(
Construct an Array from an existing NumPy ndarray.

Parameters
----------
arg0 : numpy.ndarray
    Input array to wrap.

Example
-------
.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :pyobject: test_constructorPyArray
)doc")
        .def(py::init<py::none>(), R"doc(
Construct an empty Array from ``None``.
)doc")
        .def(py::init<const std::string&>(), R"doc(
Construct a string Array from UTF-8 text.

Example
-------
.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :pyobject: test_constructorString
)doc")
        .def(py::init<const Array&>(), R"doc(
Copy-construct from another Array.
)doc")

        .def(py::init<int8_t>())
        .def(py::init<int16_t>())
        .def(py::init<int32_t>())
        .def(py::init<int64_t>())
        .def(py::init<uint8_t>())
        .def(py::init<uint16_t>())
        .def(py::init<uint32_t>())
        .def(py::init<uint64_t>())
        .def(py::init<float>())
        .def(py::init<double>())
        .def(py::init<bool>())


        .def("__getitem__", [](const Array& self, py::object key) -> Array {
            return Array(self.getPyArray().attr("__getitem__")(key));
        }, R"doc(
Return a sliced/indexed view as an :py:class:`Array`.

Parameters
----------
key : Any
    NumPy-compatible index or slice expression.

Returns
-------
Array
    Indexed value wrapped as Array.
)doc")
        .def("__setitem__", [](Array& self, py::object key, py::object value) {
            self.getPyArray().attr("__setitem__")(key, value);
        }, R"doc(
Assign a value into the underlying NumPy array.

Parameters
----------
key : Any
    NumPy-compatible index or slice.
value : Any
    Value assigned to the selected position(s).
)doc")
    
        .def("dimensions", &Array::dimensions, R"doc(
Return array rank (number of dimensions).

Returns
-------
int
    Number of dimensions.
)doc")
        .def("size", &Array::size, R"doc(
Return total number of elements.

Returns
-------
int
    Flat element count.
)doc")
        .def("shape", &Array::shape, R"doc(
Return shape as a list of dimension lengths.

Returns
-------
list[int]
    Shape vector.

Example
-------
.. literalinclude:: ../../../tests/python/array/test_array.py
   :language: python
   :pyobject: test_getArrayProperties
)doc")
        .def("strides", &Array::strides, R"doc(
Return strides (in bytes) for each dimension.

Returns
-------
list[int]
    Stride vector.
)doc")
        .def("info", &Array::info, R"doc(
Return detailed metadata and payload summary.

Returns
-------
str
    Human-readable Array description.
)doc")

        .def("getItemAtIndex", [](Array& self, const size_t& flatIndex) -> py::object {

            if      (self.hasDataOfType<int8_t>())   { return py::cast(self.getItemAtIndex<int8_t>(flatIndex));}
            else if (self.hasDataOfType<int16_t>())  { return py::cast(self.getItemAtIndex<int16_t>(flatIndex));}
            else if (self.hasDataOfType<int32_t>())  { return py::cast(self.getItemAtIndex<int32_t>(flatIndex));}
            else if (self.hasDataOfType<int64_t>())  { return py::cast(self.getItemAtIndex<int64_t>(flatIndex));}
            else if (self.hasDataOfType<uint8_t>())  { return py::cast(self.getItemAtIndex<uint8_t>(flatIndex));}
            else if (self.hasDataOfType<uint16_t>()) { return py::cast(self.getItemAtIndex<uint16_t>(flatIndex));}
            else if (self.hasDataOfType<uint32_t>()) { return py::cast(self.getItemAtIndex<uint32_t>(flatIndex));}
            else if (self.hasDataOfType<uint64_t>()) { return py::cast(self.getItemAtIndex<uint64_t>(flatIndex));}
            else if (self.hasDataOfType<float>())    { return py::cast(self.getItemAtIndex<float>(flatIndex));}
            else if (self.hasDataOfType<double>())   { return py::cast(self.getItemAtIndex<double>(flatIndex));}
            else if (self.hasDataOfType<bool>())     { return py::cast(self.getItemAtIndex<bool>(flatIndex));}
            else { throw std::runtime_error("Array::getItemAtIndex unsupported array data type"); }
        }, R"doc(
Read one element by flat index and return a Python scalar.

Parameters
----------
flatIndex : int
    Zero-based flat index.

Returns
-------
Any
    Scalar value at requested position.
)doc")


        .def("getPyArray", &Array::getPyArray, R"doc(
Expose the underlying ``numpy.ndarray`` view.

Returns
-------
numpy.ndarray
    Wrapped NumPy array.
)doc")
        .def("isNone", &Array::isNone, R"doc(
Check whether this Array is empty (None-like).

Returns
-------
bool
    ``True`` when empty.
)doc")
        .def("isScalar", &Array::isScalar, R"doc(
Check whether this Array stores one scalar numeric value.

Returns
-------
bool
    ``True`` for scalar payloads.
)doc")
        .def("isContiguous", &Array::isContiguous, R"doc(
Check whether memory layout is contiguous (C or Fortran order).

Returns
-------
bool
    ``True`` when contiguous.
)doc")
        .def("isContiguousInStyleC", &Array::isContiguousInStyleC, R"doc(
Check C-order (row-major) contiguity.

Returns
-------
bool
    ``True`` for C-order contiguous arrays.
)doc")
        .def("isContiguousInStyleFortran", &Array::isContiguousInStyleFortran, R"doc(
Check Fortran-order (column-major) contiguity.

Returns
-------
bool
    ``True`` for Fortran-order contiguous arrays.
)doc")
        
        .def("hasString", &Array::hasString, R"doc(
Check whether this Array stores string data.

Returns
-------
bool
    ``True`` for string payloads.
)doc")
        .def("print", &Array::print, R"doc(
Print a compact textual representation.

Parameters
----------
arg0 : int
    Maximum characters to emit.
)doc")
        .def("getPrintString", &Array::getPrintString, R"doc(
Return a compact textual representation.

Parameters
----------
arg0 : int
    Maximum characters in output.

Returns
-------
str
    Formatted representation.
)doc")

        .def("extractString", &Array::extractString, R"doc(
Decode and return string payload.

Returns
-------
str
    UTF-8 string value.

Example
-------
.. literalinclude:: ../../../tests/python/array/factory/test_strings.py
   :language: python
   :pyobject: test_arrayFromString
)doc")
        
        .def("getFlatIndex", &Array::getFlatIndex, R"doc(
Convert multidimensional indices to one flat index.

Parameters
----------
arg0 : list[int]
    Indices per dimension.

Returns
-------
int
    Flat index in the current memory order.
)doc")

        .def("__repr__", [](const Array& self) {
            std::ostringstream repr;
            repr << "Array(" << self.getPyArray().attr("__repr__")().cast<std::string>() << ")";
            return repr.str();
        }, R"doc(
Return ``repr(array)`` style representation.
)doc")
    ;

    bindFactoryOfArrays(m);
}
