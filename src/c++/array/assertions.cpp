# include "array/assertions.hpp"

Array::Assertions::Assertions(const Array& inputArray) : array(inputArray) {}

template <typename T>
void Array::Assertions::haveValidDataTypeForSettingScalar() const {
    try {
        this->haveDataOfType<T>();
    } catch (const py::type_error& e) {
        if (array.hasString()) {
            throw py::type_error("cannot assign a scalar to an array containing a string");
        }
        throw e;
    }
}

/*
    template instantiations
*/

template <typename... T>
struct Instantiator {
    template <typename... U>
    void operator()() const {
        const Array constArray;
        auto assertor = constArray.must();
        (static_cast<void>(assertor.haveValidDataTypeForSettingScalar<U>()), ...);
    }
};

template void utils::instantiateFromTypeList<Instantiator, utils::ScalarTypes>();
