# ifndef DATA_FACTORY_HPP
# define DATA_FACTORY_HPP

# include <memory>
# include <string>

# include "data/data.hpp"

namespace datafactory {

template <typename T>
std::shared_ptr<Data> makeDataFrom(const T& value);

std::shared_ptr<Data> makeDataFrom(const char* value);

}

void registerDefaultFactory();
void ensureFactoryInitialized();

# endif
