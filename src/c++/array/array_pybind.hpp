# include "array/array.hpp"
# include "array/factory/factory_pybind.hpp"

void bindArray(py::module_ &m) {

    py::class_<Array, Data, std::shared_ptr<Array>>(m, "Array")

        .def(py::init<>())
        .def(py::init<const py::array&>())
        .def(py::init<py::none>())
        .def(py::init<const std::string&>())
        .def(py::init<const Array&>())

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
        })
        .def("__setitem__", [](Array& self, py::object key, py::object value) {
            self.getPyArray().attr("__setitem__")(key, value);
        })
    
        .def("dimensions", &Array::dimensions)
        .def("size", &Array::size)
        .def("shape", &Array::shape)
        .def("strides", &Array::strides)
        .def("info", &Array::info)

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
        })


        .def("getPyArray", &Array::getPyArray)
        .def("isNone", &Array::isNone)
        .def("isScalar", &Array::isScalar)
        .def("isContiguous", &Array::isContiguous)
        .def("isContiguousInStyleC", &Array::isContiguousInStyleC)
        .def("isContiguousInStyleFortran", &Array::isContiguousInStyleFortran)
        
        .def("hasString", &Array::hasString)
        .def("print", &Array::print)
        .def("getPrintString", &Array::getPrintString)

        .def("extractString", &Array::extractString)
        
        .def("getFlatIndex", &Array::getFlatIndex)

        .def("__repr__", [](const Array& self) {
            std::ostringstream repr;
            repr << "Array(" << self.getPyArray().attr("__repr__")().cast<std::string>() << ")";
            return repr.str();
        })
    ;

    bindFactoryOfArrays(m);
}
