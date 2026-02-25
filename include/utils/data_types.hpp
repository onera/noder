# ifndef DATA_TYPES_HPP
# define DATA_TYPES_HPP

# include <cstdint>
# include <string>
#include <string_view>
#include <type_traits>

namespace utils {

    /** @brief Human-readable short names for scalar types used in bindings. */
    // Helper to get human-readable type names
    template <typename T>
    constexpr std::string_view getTypeName() {
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

    /** @brief Tag wrapper used by generic bind/dispatch helpers. */
    template <typename T>
    struct TypeTag { using type = T; };

    /** @brief Compile-time list of types. */
    template <typename... T>
    struct TypeList {};

    template <typename T>
    using DecayCvRef_t = std::remove_cv_t<std::remove_reference_t<T>>;

    /** @brief Concatenate two TypeList values. */
    template <typename List1, typename List2>
    struct Concat;

    template <typename... T1, typename... T2>
    struct Concat<TypeList<T1...>, TypeList<T2...>> {
        using type = TypeList<T1..., T2...>;
    };

    /** @brief Alias for concatenated TypeList type. */
    template <typename List1, typename List2>
    using Concat_t = typename Concat<List1, List2>::type;


    using FloatingTypes = TypeList<float, double>;

    using PositiveIntegralTypes = TypeList<uint8_t, uint16_t, uint32_t, uint64_t>;
    
    using SignedIntegralTypes = TypeList<int8_t, int16_t, int32_t, int64_t>;

    using IntegralTypes = Concat_t<SignedIntegralTypes, PositiveIntegralTypes>;

    using FloatingAndIntegralTypes = Concat_t<FloatingTypes, IntegralTypes>;

    using ScalarTypes = Concat_t<TypeList<bool>, FloatingAndIntegralTypes>;

    using StringAndScalarTypes = Concat_t<TypeList<std::string>, ScalarTypes>;

    /**
     * @brief Preferred dispatch order for Python runtime integral conversions.
     *
     * Wider types are attempted first.
     */
    using PythonIntegralDispatchTypes = TypeList<
        int64_t, uint64_t,
        int32_t, uint32_t,
        int16_t, uint16_t,
        int8_t, uint8_t>;

    /** @brief Preferred dispatch order for Python runtime floating conversions. */
    using PythonFloatingDispatchTypes = TypeList<double, float>;

    /** @brief True when T appears in TypeList TL (after cv-ref decay). */
    template <typename T, typename TL>
    struct ContainsType;

    template <typename T, typename... U>
    struct ContainsType<T, TypeList<U...>> :
        std::bool_constant<(std::is_same_v<DecayCvRef_t<T>, U> || ...)> {};

    template <typename T, typename TL>
    inline constexpr bool ContainsType_v = ContainsType<T, TL>::value;

    /** @brief Canonical scalar type used for safe runtime dispatching. */
    template <typename T, typename Enable = void>
    struct CanonicalScalarType;

    template <typename T>
    struct CanonicalScalarType<T, std::enable_if_t<std::is_arithmetic_v<DecayCvRef_t<T>>>> {
        using U = DecayCvRef_t<T>;
        using type = std::conditional_t<
            std::is_same_v<U, bool>,
            bool,
            std::conditional_t<
                std::is_floating_point_v<U>,
                std::conditional_t<(sizeof(U) <= sizeof(float)), float, double>,
                std::conditional_t<
                    std::is_signed_v<U>,
                    std::conditional_t<
                        (sizeof(U) <= sizeof(int8_t)),
                        int8_t,
                        std::conditional_t<
                            (sizeof(U) <= sizeof(int16_t)),
                            int16_t,
                            std::conditional_t<
                                (sizeof(U) <= sizeof(int32_t)),
                                int32_t,
                                int64_t
                            >
                        >
                    >,
                    std::conditional_t<
                        (sizeof(U) <= sizeof(uint8_t)),
                        uint8_t,
                        std::conditional_t<
                            (sizeof(U) <= sizeof(uint16_t)),
                            uint16_t,
                            std::conditional_t<
                                (sizeof(U) <= sizeof(uint32_t)),
                                uint32_t,
                                uint64_t
                            >
                        >
                    >
                >
            >
        >;
    };

    template <typename T>
    using CanonicalScalar_t = typename CanonicalScalarType<T>::type;
} 

# endif
