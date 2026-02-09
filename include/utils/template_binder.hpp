# ifndef TEMPLATE_BINDER_HPP
# define TEMPLATE_BINDER_HPP

# include <pybind11/pybind11.h>
# include <string>
# include <type_traits>
# include <utility>

# include "data_types.hpp"

namespace py = pybind11;

namespace utils {


// Generic function binder for all types in a TypeList
template <typename FunctionTemplate, typename... T>
void bindForSpecifiedTypeList(
    py::module_ &m,
    const std::string &baseName,
    TypeList<T...>,
    FunctionTemplate&& functionTemplate) {
    (m.def(
        (baseName + "_" + std::string(getTypeName<T>())).c_str(),
        std::forward<FunctionTemplate>(functionTemplate).template operator()<T>(TypeTag<T>{})
     ),
     ...);
}

// Helpers to bind functions for predefined TypeList (see data_types.hpp)
template <typename FunctionTemplate>
void bindForFloatingTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, FloatingTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForPositiveIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, PositiveIntegralTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForSignedIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, SignedIntegralTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, IntegralTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForFloatingAndIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, FloatingAndIntegralTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForScalarTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, ScalarTypes{}, std::forward<FunctionTemplate>(ft));
}

template <typename FunctionTemplate>
void bindForStringAndScalarTypes(py::module_ &m, const std::string &baseName, FunctionTemplate&& ft) {
    bindForSpecifiedTypeList(m, baseName, StringAndScalarTypes{}, std::forward<FunctionTemplate>(ft));
}

} 



# endif
