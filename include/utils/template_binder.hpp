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
    auto&& ft = functionTemplate;
    (m.def(
        (baseName + "_" + std::string(getTypeName<T>())).c_str(),
        ft.template operator()<T>(TypeTag<T>{})
     ),
     ...);
}

// Generic class-method binder for all types in a TypeList
template <typename ClassBinding, typename MethodTemplate, typename... T, typename... DefArgs>
void bindClassMethodForSpecifiedTypeList(
    ClassBinding &cls,
    const std::string &baseName,
    TypeList<T...>,
    MethodTemplate&& methodTemplate,
    DefArgs&&... defArgs) {
    auto&& mt = methodTemplate;
    (cls.def(
        (baseName + "_" + std::string(getTypeName<T>())).c_str(),
        mt.template operator()<T>(TypeTag<T>{}),
        std::forward<DefArgs>(defArgs)...
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

// Helpers to bind class methods for predefined TypeList (see data_types.hpp)
template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForFloatingTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, FloatingTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForPositiveIntegralTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, PositiveIntegralTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForSignedIntegralTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, SignedIntegralTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForIntegralTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, IntegralTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForFloatingAndIntegralTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, FloatingAndIntegralTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForScalarTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, ScalarTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

template <typename ClassBinding, typename MethodTemplate, typename... DefArgs>
void bindClassMethodForStringAndScalarTypes(
    ClassBinding &cls,
    const std::string &baseName,
    MethodTemplate&& mt,
    DefArgs&&... defArgs) {
    bindClassMethodForSpecifiedTypeList(
        cls, baseName, StringAndScalarTypes{}, std::forward<MethodTemplate>(mt), std::forward<DefArgs>(defArgs)...);
}

} 



# endif
