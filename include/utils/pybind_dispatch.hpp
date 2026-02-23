# ifndef PYBIND_DISPATCH_HPP
# define PYBIND_DISPATCH_HPP

# include <utility>

# include <pybind11/pybind11.h>

# include "data_types.hpp"

namespace py = pybind11;

namespace utils {

struct PyCastDispatchInfo {
    bool anyCast = false;
    bool stopped = false;
};

inline bool isRecoverablePyConversionError(const py::error_already_set& e) {
    return e.matches(PyExc_TypeError) ||
           e.matches(PyExc_OverflowError) ||
           e.matches(PyExc_ValueError);
}

template <typename... T, typename Visitor>
PyCastDispatchInfo dispatchByCastedTypeList(
    const py::object& data,
    TypeList<T...>,
    Visitor&& visitor) {

    PyCastDispatchInfo info;

    auto tryType = [&]<typename U>() {
        if (info.stopped) {
            return;
        }
        try {
            U value = data.cast<U>();
            info.anyCast = true;
            info.stopped = visitor.template operator()<U>(value);
        } catch (const py::cast_error&) {
            return;
        } catch (const py::error_already_set& e) {
            if (isRecoverablePyConversionError(e)) {
                return;
            }
            throw;
        }
    };

    (tryType.template operator()<T>(), ...);
    return info;
}

inline bool tryUnwrapPyScalarItem(const py::object& data, py::object& scalar) {
    if (!py::hasattr(data, "item")) {
        return false;
    }
    try {
        scalar = data.attr("item")();
        return scalar.ptr() != data.ptr();
    } catch (const py::error_already_set&) {
        return false;
    }
}

}

# endif
