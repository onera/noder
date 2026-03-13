#include "array/factory/vectors.hpp"

#include "array/factory/matrices.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace arrayfactory {

    template <typename T>
    Array uniformFromStep(const double& start, const double& stop, const double& step) {
        constexpr double epsilon = std::numeric_limits<double>::epsilon();

        if (std::abs(step) < epsilon) {
            throw std::invalid_argument("step must not be zero");
        }

        if (((step < 0) && (start < stop)) || ((step > 0) && (start > stop))) {
            throw std::invalid_argument("incoherent set of start, stop, step");
        }

        const ssize_t signedSize = static_cast<ssize_t>((stop - start) / step);
        Array array = empty<T>({static_cast<size_t>(signedSize)}, 'C');
        T* buffer = array.getPointerOfModifiableDataFast<T>();
        for (size_t i = 0; i < static_cast<size_t>(signedSize); ++i) {
            buffer[i] = static_cast<T>(start + static_cast<double>(i) * step);
        }

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

        Array array = empty<T>({num}, 'C');
        T* buffer = array.getPointerOfModifiableDataFast<T>();
        const double floatNum = static_cast<double>(num);
        const double step = (endpoint && floatNum > 1) ? (stop - start) / (floatNum - 1) : (stop - start) / floatNum;

        for (size_t i = 0; i < num; ++i) {
            buffer[i] = static_cast<T>(start + static_cast<double>(i) * step);
        }
        if (endpoint && num > 1) {
            buffer[num - 1] = static_cast<T>(stop);
        }
        return array;
    }
}

template <typename... T>
struct InstantiatorFloatingAndIntegralTypes {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&arrayfactory::uniformFromStep<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorFloatingAndIntegralTypes,
                                             utils::FloatingAndIntegralTypes>();

template <typename... T>
struct InstantiatorFloatingTypes {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(&arrayfactory::uniformFromCount<U>), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorFloatingTypes, utils::FloatingTypes>();
