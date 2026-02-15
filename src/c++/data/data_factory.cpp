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

// One-time initialization
void ensureFactoryInitialized() {
    static bool initialized = false;
    if (!initialized) {
        registerDefaultFactory();
        initialized = true;
    }
}

// GUARANTEE initialization at library load-time using global static initialization
struct AutoInitializer {
    AutoInitializer() {
        ensureFactoryInitialized();
    }
};

// This global object forces initialization at shared library load
static AutoInitializer autoInit;
extern "C" volatile auto* force_auto_init = &autoInit; // ensures no dead-stripping


template <typename T>
std::shared_ptr<Data> datafactory::makeDataFrom(const T& value) {
    return std::make_shared<Array>(value);
}

std::shared_ptr<Data> datafactory::makeDataFrom(const char* value) {
    return std::make_shared<Array>(value);
}

template <typename... T>
struct InstantiatorDataFactory {
    template <typename... U>
    void operator()() const {
        (utils::forceSymbol(static_cast<std::shared_ptr<Data>(*)(const U&)>(&datafactory::makeDataFrom<U>)), ...);
    }
};

template void utils::instantiateFromTypeList<InstantiatorDataFactory, utils::StringAndScalarTypes>();
