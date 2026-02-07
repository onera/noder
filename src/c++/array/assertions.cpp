# include "array/assertions.hpp"

Array::Assertions::Assertions(const Array& inputArray) : array(inputArray) {}

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
