# ifndef DATA_FACTORY_HPP
# define DATA_FACTORY_HPP

# include <memory>
# include <string>

# include "data/data.hpp"

namespace datafactory {

/**
 * @brief Convert a C++ value into the default Data implementation.
 * @tparam T Source value type.
 * @param value Input value.
 * @return Shared payload instance that wraps the value.
 */
template <typename T>
std::shared_ptr<Data> makeDataFrom(const T& value);

/** @brief C-string convenience overload of makeDataFrom. */
std::shared_ptr<Data> makeDataFrom(const char* value);
/** @brief Build the default empty payload used for uninitialized nodes. */
std::shared_ptr<Data> makeDefaultData();

}

/** @brief Register the project default payload factory. */
void registerDefaultFactory();
/** @brief Ensure default payload factory is initialized once. */
void ensureFactoryInitialized();

# endif
