# ifndef UTILS_COMPARATOR_HPP
# define UTILS_COMPARATOR_HPP

# include <vector>

namespace utils {

    template <typename T>
    bool approxEqual(const T& a, const T& b) {
        if constexpr (std::is_same_v<T, float>) {
            constexpr float ε = std::numeric_limits<float>::epsilon();
            return std::abs(a-b) < ε;
        }
        else if constexpr (std::is_same_v<T, double>) {
            constexpr double ε = std::numeric_limits<double>::epsilon();
            return std::abs(a-b) < ε;
        }
        else {
            return a == b;
        }
    };

    template <typename T>
    bool approxEqual(const T& a, typename std::vector<T>::reference b) {
        const T& B = static_cast<const T&>(b); 
        return approxEqual(a,B);
    }

}

# endif