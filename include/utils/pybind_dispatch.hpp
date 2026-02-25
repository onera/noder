# ifndef PYBIND_DISPATCH_HPP
# define PYBIND_DISPATCH_HPP

# include <utility>

# include <pybind11/pybind11.h>

# include "data_types.hpp"

namespace py = pybind11;

namespace utils {

/**
 * @brief Outcome flags returned by runtime cast dispatch helper.
 */
struct PyCastDispatchInfo {
    bool anyCast = false;
    bool stopped = false;
};

/** @brief True for recoverable Python conversion errors during probing. */
inline bool isRecoverablePyConversionError(const py::error_already_set& e) {
    return e.matches(PyExc_TypeError) ||
           e.matches(PyExc_OverflowError) ||
           e.matches(PyExc_ValueError);
}

template <typename... T, typename Visitor>
/**
 * @brief Try casting @p data to each type in a TypeList and visit successful casts.
 *
 * Stops early when visitor returns true.
 */
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

/** @brief Try `data.item()` to unwrap NumPy scalar wrappers. */
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
