# ifndef TEMPLATE_INSTANTIATOR_HPP
# define TEMPLATE_INSTANTIATOR_HPP

# include <cstdint>
# include <vector>
# include <string>

# include "data_types.hpp"

namespace utils {

    // Force ODR-use of function/member-function pointers in a portable way.
    // This prevents optimized builds from discarding template instantiations.
    template <typename SymbolPtr>
    inline void forceSymbol(SymbolPtr symbol) {
        volatile SymbolPtr sink = symbol;
        (void)sink;
    }

    // General instantiation function for an instantiator and a TypeList
    template <template <typename...> class Instantiator, typename... T>
    void instantiate(utils::TypeList<T...>) {
        Instantiator<T...> callable;
        callable.template operator()<T...>();
    }

    // Direct instantiation for variadic types
    template <template <typename...> class Instantiator, typename... T>
    void instantiate() {
        Instantiator<T...> callable;
        callable.template operator()<T...>();
    }

    // Helper to unpack TypeList and call instantiate
    template <template <typename...> class Instantiator, typename TypeList>
    struct InstantiateHelper;

    template <template <typename...> class Instantiator, typename... T>
    struct InstantiateHelper<Instantiator, TypeList<T...>> {
        static void instantiate() {
            utils::instantiate<Instantiator, T...>();
        }
    };

    // Convenient function for unpacking TypeList
    template <template <typename...> class Instantiator, typename TypeList>
    void instantiateFromTypeList() {
        InstantiateHelper<Instantiator, TypeList>::instantiate();
    }
} 

# endif 
