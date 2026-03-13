#ifndef ARRAY_NUMPY_BRIDGE_HPP
#define ARRAY_NUMPY_BRIDGE_HPP

#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "array/array.hpp"

namespace py = pybind11;

namespace arraybridge {

inline std::vector<size_t> toUnsignedVector(const std::vector<ssize_t>& values) {
    std::vector<size_t> output;
    output.reserve(values.size());
    for (ssize_t value : values) {
        if (value < 0) {
            throw std::overflow_error("Negative ssize_t cannot be converted to size_t");
        }
        output.push_back(static_cast<size_t>(value));
    }
    return output;
}

inline std::vector<ssize_t> toSignedVector(const std::vector<size_t>& values) {
    std::vector<ssize_t> output;
    output.reserve(values.size());
    for (size_t value : values) {
        if (value > static_cast<size_t>(std::numeric_limits<ssize_t>::max())) {
            throw std::overflow_error("size_t value exceeds ssize_t range");
        }
        output.push_back(static_cast<ssize_t>(value));
    }
    return output;
}

inline ArrayTypeId typeIdFromPyArray(const py::array& array) {
    const py::dtype dtype = array.dtype();
    const char kind = dtype.kind();

    if (dtype.is(py::dtype::of<bool>())) return ArrayTypeId::Bool;
    if (dtype.is(py::dtype::of<int8_t>())) return ArrayTypeId::Int8;
    if (dtype.is(py::dtype::of<int16_t>())) return ArrayTypeId::Int16;
    if (dtype.is(py::dtype::of<int32_t>())) return ArrayTypeId::Int32;
    if (dtype.is(py::dtype::of<int64_t>())) return ArrayTypeId::Int64;
    if (dtype.is(py::dtype::of<uint8_t>())) return ArrayTypeId::UInt8;
    if (dtype.is(py::dtype::of<uint16_t>())) return ArrayTypeId::UInt16;
    if (dtype.is(py::dtype::of<uint32_t>())) return ArrayTypeId::UInt32;
    if (dtype.is(py::dtype::of<uint64_t>())) return ArrayTypeId::UInt64;
    if (dtype.is(py::dtype::of<float>())) return ArrayTypeId::Float32;
    if (dtype.is(py::dtype::of<double>())) return ArrayTypeId::Float64;
    if (kind == 'S') return ArrayTypeId::Bytes;
    if (kind == 'U') return ArrayTypeId::Unicode;
    if (kind == 'O') return ArrayTypeId::None;

    throw py::type_error("Unsupported NumPy dtype for Array");
}

inline std::shared_ptr<void> ownerFromPyArray(const py::array& array) {
    return std::shared_ptr<void>(new py::array(array), [](void* ptr) {
        py::gil_scoped_acquire acquireGil;
        delete static_cast<py::array*>(ptr);
    });
}

inline Array arrayFromPyArray(const py::array& array) {
    const ArrayTypeId typeId = typeIdFromPyArray(array);

    if (typeId == ArrayTypeId::None) {
        const ssize_t size = array.size();
        if (size == 0) {
            return Array();
        }
        if (size == 1) {
            py::object flat = array.attr("flat");
            py::object value = flat.attr("__getitem__")(0);
            if (value.is_none()) {
                return Array();
            }
        }
        throw py::type_error("Only NumPy object arrays representing None are supported");
    }

    const py::buffer_info info = array.request();

    return Array(
        typeId,
        static_cast<size_t>(info.itemsize),
        info.ptr,
        toUnsignedVector(info.shape),
        toUnsignedVector(info.strides),
        ownerFromPyArray(array),
        ArrayOwnerKind::PythonArray);
}

inline Array arrayFromPyObject(const py::object& objectValue) {
    if (objectValue.is_none()) {
        return Array();
    }
    if (py::isinstance<Array>(objectValue)) {
        return objectValue.cast<Array>();
    }
    if (py::isinstance<py::array>(objectValue)) {
        return arrayFromPyArray(objectValue.cast<py::array>());
    }
    if (py::hasattr(objectValue, "dtype")) {
        py::module_ np = py::module_::import("numpy");
        return arrayFromPyArray(np.attr("asarray")(objectValue).cast<py::array>());
    }
    if (py::isinstance<py::str>(objectValue)) {
        return Array(objectValue.cast<std::string>());
    }
    if (py::isinstance<py::bool_>(objectValue)) {
        return Array(objectValue.cast<bool>());
    }
    if (py::isinstance<py::int_>(objectValue)) {
        return Array(objectValue.cast<int64_t>());
    }
    if (py::isinstance<py::float_>(objectValue)) {
        return Array(objectValue.cast<double>());
    }

    try {
        py::module_ np = py::module_::import("numpy");
        return arrayFromPyArray(np.attr("asarray")(objectValue).cast<py::array>());
    } catch (const py::error_already_set&) {
        throw py::type_error("Object could not be converted to Array");
    }
}

inline py::dtype dtypeFromArray(const Array& array) {
    switch (array.typeId()) {
        case ArrayTypeId::Bool: return py::dtype::of<bool>();
        case ArrayTypeId::Int8: return py::dtype::of<int8_t>();
        case ArrayTypeId::Int16: return py::dtype::of<int16_t>();
        case ArrayTypeId::Int32: return py::dtype::of<int32_t>();
        case ArrayTypeId::Int64: return py::dtype::of<int64_t>();
        case ArrayTypeId::UInt8: return py::dtype::of<uint8_t>();
        case ArrayTypeId::UInt16: return py::dtype::of<uint16_t>();
        case ArrayTypeId::UInt32: return py::dtype::of<uint32_t>();
        case ArrayTypeId::UInt64: return py::dtype::of<uint64_t>();
        case ArrayTypeId::Float32: return py::dtype::of<float>();
        case ArrayTypeId::Float64: return py::dtype::of<double>();
        case ArrayTypeId::Bytes: return py::dtype("S" + std::to_string(array.itemsize()));
        case ArrayTypeId::Unicode: return py::dtype("U" + std::to_string(array.itemsize() / sizeof(char32_t)));
        case ArrayTypeId::None: break;
    }
    throw py::type_error("Cannot convert None-like Array to NumPy dtype");
}

inline py::object baseObjectFromOwner(const std::shared_ptr<void>& owner) {
    if (!owner) {
        return py::none();
    }

    auto* holder = new std::shared_ptr<void>(owner);
    return py::capsule(holder, [](void* ptr) {
        delete static_cast<std::shared_ptr<void>*>(ptr);
    });
}

inline py::object toPyObject(const Array& array) {
    if (array.typeId() == ArrayTypeId::None) {
        return py::none();
    }

    if (array.ownerKind() == ArrayOwnerKind::PythonArray && array.owner()) {
        py::array original = *static_cast<py::array*>(array.owner().get());
        py::buffer_info info = original.request();
        if (info.ptr == array.rawData() &&
            toUnsignedVector(info.shape) == array.shape() &&
            toUnsignedVector(info.strides) == array.strides()) {
            return original;
        }

        return py::array(
            dtypeFromArray(array),
            toSignedVector(array.shape()),
            toSignedVector(array.strides()),
            array.rawData(),
            original);
    }

    return py::array(
        dtypeFromArray(array),
        toSignedVector(array.shape()),
        toSignedVector(array.strides()),
        array.rawData(),
        baseObjectFromOwner(array.owner()));
}

inline py::array toPyArray(const Array& array) {
    return toPyObject(array).cast<py::array>();
}

} // namespace arraybridge

#endif
