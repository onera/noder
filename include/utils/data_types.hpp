# ifndef DATA_TYPES_HPP
# define DATA_TYPES_HPP

# include <cstdint>
# include <string>

namespace utils {

    // Helper to get human-readable type names
    template <typename T>
    constexpr std::string getTypeName() {
        if constexpr (std::is_same_v<T, bool>) return "bool";
        else if constexpr (std::is_same_v<T, int8_t>) return "int8";
        else if constexpr (std::is_same_v<T, int16_t>) return "int16";
        else if constexpr (std::is_same_v<T, int32_t>) return "int32";
        else if constexpr (std::is_same_v<T, int64_t>) return "int64";
        else if constexpr (std::is_same_v<T, uint8_t>) return "uint8";
        else if constexpr (std::is_same_v<T, uint16_t>) return "uint16";
        else if constexpr (std::is_same_v<T, uint32_t>) return "uint32";
        else if constexpr (std::is_same_v<T, uint64_t>) return "uint64";
        else if constexpr (std::is_same_v<T, float>) return "float";
        else if constexpr (std::is_same_v<T, double>) return "double";
        else return "unknown";
    }

    // TypeList for managing variadic type packs
    template <typename... T>
    struct TypeList {};

    // Concatenation utility
    template <typename List1, typename List2>
    struct Concat;

    template <typename... T1, typename... T2>
    struct Concat<TypeList<T1...>, TypeList<T2...>> {
        using type = TypeList<T1..., T2...>;
    };

    // Helper alias for easier usage
    template <typename List1, typename List2>
    using Concat_t = typename Concat<List1, List2>::type;


    using FloatingTypes = TypeList<float, double>;

    using PositiveIntegralTypes = TypeList<uint8_t, uint16_t, uint32_t, uint64_t>;
    
    using SignedIntegralTypes = TypeList<int8_t, int16_t, int32_t, int64_t>;

    using IntegralTypes = Concat_t<SignedIntegralTypes, PositiveIntegralTypes>;

    using FloatingAndIntegralTypes = Concat_t<FloatingTypes, IntegralTypes>;

    using ScalarTypes = Concat_t<TypeList<bool>, FloatingAndIntegralTypes>;

    using StringAndScalarTypes = Concat_t<TypeList<std::string>, ScalarTypes>;
} 

# endif 