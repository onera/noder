// data_factory.cpp

#include "data/data_factory.hpp"
#include "array/array.hpp"
#include "node/node.hpp"
#include "utils/data_types.hpp"
#include "utils/template_instantiator.hpp"
#include <memory>

void registerDefaultFactory() {
    Node::setDefaultFactory([]() { return std::make_shared<Array>(); });
}

// One-time initialization on first use (thread-safe since C++11 local static initialization)
void ensureFactoryInitialized() {
    static const bool initialized = []() {
        registerDefaultFactory();
        return true;
    }();
    (void)initialized;
}


template <typename T>
std::shared_ptr<Data> datafactory::makeDataFrom(const T& value) {
    return std::make_shared<Array>(value);
}

std::shared_ptr<Data> datafactory::makeDataFrom(const char* value) {
    return std::make_shared<Array>(value);
}

std::shared_ptr<Data> datafactory::makeDefaultData() {
    auto probeNode = std::make_shared<Node>();
    return probeNode->data().clone();
}

template <typename... T>
struct InstantiatorDataFactory {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(static_cast<std::shared_ptr<Data>(*)(const U&)>(&datafactory::makeDataFrom<U>)), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorDataFactory, utils::StringAndScalarTypes>();
