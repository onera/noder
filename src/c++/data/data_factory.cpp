// data_factory.cpp

#include "data/data_factory.hpp"
#include "array/array.hpp"
#include "node/node.hpp"
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
