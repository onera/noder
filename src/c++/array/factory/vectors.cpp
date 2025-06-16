# include "array/factory/vectors.hpp"

namespace arrayfactory {

    template <typename T>
    Array uniformFromStep(const double& start, const double& stop, const double& step) {
        constexpr double ε = std::numeric_limits<double>::epsilon();

        if (std::abs(step) < ε) {
            throw std::invalid_argument("step must not be zero");
        }

        if (((step < 0) && (start < stop)) || ((step > 0) && (start > stop)) ) {
            throw std::invalid_argument("incoherent set of start, stop, step");
        }


        ssize_t ssize = static_cast<ssize_t>((stop - start) / step);
        py::array_t<T> pyarray(ssize);

        T* buffer = static_cast<T*>(pyarray.mutable_data());
        for (size_t i = 0; i < static_cast<size_t>(ssize); ++i) {
            buffer[i] = static_cast<T>(start + static_cast<double>(i) * step);
        }

        Array array(pyarray);

        return array;
    }

    template <typename T>
    Array uniformFromCount(const double& start,
                   const double& stop,
                   const size_t& num,
                   const bool& endpoint) {

        if (num < 2) { 
            throw std::invalid_argument("num must be at least 2");
        }

        py::array_t<T> pyarray(static_cast<ssize_t>(num));
        T* buffer = static_cast<T*>(pyarray.mutable_data());
        double floatnum = static_cast<double>(num);
        double step = (endpoint && floatnum > 1) ? (stop - start) / (floatnum - 1) : (stop - start) / floatnum;

        for (size_t i = 0; i < num; ++i) {
            buffer[i] = static_cast<T>(start + static_cast<double>(i) * step);
        }
        if (endpoint && num > 1) {
            buffer[num - 1] = static_cast<T>(stop);
        }
        Array array(pyarray);
        return array;
    }
} 

/*
    templates instantiations
*/

template <typename... T>
struct InstantiatorFloatingAndIntegralTypes {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(arrayfactory::uniformFromStep<U>(T{}, T{}, T{})), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorFloatingAndIntegralTypes,
                                                  utils::FloatingAndIntegralTypes>();

template <typename... T>
struct InstantiatorFloatingTypes {
    template <typename... U>
    void operator()() const {
        (static_cast<void>(arrayfactory::uniformFromCount<U>(T{}, T{}, size_t{}, bool{})), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorFloatingTypes,
                                                  utils::FloatingTypes>();
