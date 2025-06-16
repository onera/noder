# ifndef TEMPLATE_BINDER_HPP
# define TEMPLATE_BINDER_HPP

# include <pybind11/pybind11.h>
# include <string>
# include <type_traits>

# include "data_types.hpp"

namespace py = pybind11;

namespace utils {


// Generic function binder for all types in a TypeList
template <typename... T>
void bindForSpecifiedTypeList(py::module_ &m, const std::string &baseName, TypeList<T...>, auto functionTemplate) {
    (m.def((baseName + "_" + std::string(getTypeName<T>())).c_str(), functionTemplate.template operator()<T>()), ...);
}

// Helpers to bind functions for predefined TypeList (see data_types.hpp)
template <typename FunctionTemplate>
void bindForFloatingTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, FloatingTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForPositiveIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, PositiveIntegralTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForSignedIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, SignedIntegralTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, IntegralTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForFloatingAndIntegralTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, FloatingAndIntegralTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForScalarTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, ScalarTypes{}, functionTemplate);
}

template <typename FunctionTemplate>
void bindForStringAndScalarTypes(py::module_ &m, const std::string &baseName, FunctionTemplate functionTemplate) {
    bindForSpecifiedTypeList(m, baseName, StringAndScalarTypes{}, functionTemplate);
}

} 



# endif
