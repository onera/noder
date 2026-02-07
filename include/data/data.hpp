# ifndef DATA_HPP
# define DATA_HPP

# include <string>
# include <memory>
# include <cstdint>
# include <type_traits>

class Data {

public:
    virtual ~Data() = default;

    virtual std::shared_ptr<Data> clone() const = 0;
    
    virtual bool hasString() const = 0;
    virtual bool isNone() const = 0;
    virtual bool isScalar() const = 0;

    virtual std::string extractString() const = 0;

    virtual std::string info() const = 0;
    virtual std::string shortInfo() const = 0;

    virtual bool operator==(const int8_t& scalar) const = 0;
    virtual bool operator==(const int16_t& scalar) const = 0;
    virtual bool operator==(const int32_t& scalar) const = 0;
    virtual bool operator==(const int64_t& scalar) const = 0;
    virtual bool operator==(const uint8_t& scalar) const = 0;
    virtual bool operator==(const uint16_t& scalar) const = 0;
    virtual bool operator==(const uint32_t& scalar) const = 0;
    virtual bool operator==(const uint64_t& scalar) const = 0;
    virtual bool operator==(const float& scalar) const = 0;
    virtual bool operator==(const double& scalar) const = 0;
    virtual bool operator==(const bool& scalar) const = 0;

    virtual bool operator!=(const int8_t& scalar) const = 0;
    virtual bool operator!=(const int16_t& scalar) const = 0;
    virtual bool operator!=(const int32_t& scalar) const = 0;
    virtual bool operator!=(const int64_t& scalar) const = 0;
    virtual bool operator!=(const uint8_t& scalar) const = 0;
    virtual bool operator!=(const uint16_t& scalar) const = 0;
    virtual bool operator!=(const uint32_t& scalar) const = 0;
    virtual bool operator!=(const uint64_t& scalar) const = 0;
    virtual bool operator!=(const float& scalar) const = 0;
    virtual bool operator!=(const double& scalar) const = 0;
    virtual bool operator!=(const bool& scalar) const = 0;

    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<std::remove_cv_t<T>>, int> = 0>
    bool operator==(const T& scalar) const {
        if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>) {
            return this->operator==(static_cast<bool>(scalar));
        } else if constexpr (std::is_floating_point_v<std::remove_cv_t<T>>) {
            if constexpr (sizeof(T) <= sizeof(float)) {
                return this->operator==(static_cast<float>(scalar));
            } else {
                return this->operator==(static_cast<double>(scalar));
            }
        } else if constexpr (std::is_signed_v<std::remove_cv_t<T>>) {
            if constexpr (sizeof(T) <= sizeof(int8_t)) {
                return this->operator==(static_cast<int8_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(int16_t)) {
                return this->operator==(static_cast<int16_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(int32_t)) {
                return this->operator==(static_cast<int32_t>(scalar));
            } else {
                return this->operator==(static_cast<int64_t>(scalar));
            }
        } else {
            if constexpr (sizeof(T) <= sizeof(uint8_t)) {
                return this->operator==(static_cast<uint8_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(uint16_t)) {
                return this->operator==(static_cast<uint16_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(uint32_t)) {
                return this->operator==(static_cast<uint32_t>(scalar));
            } else {
                return this->operator==(static_cast<uint64_t>(scalar));
            }
        }
    }

    template <typename T, typename std::enable_if_t<std::is_arithmetic_v<std::remove_cv_t<T>>, int> = 0>
    bool operator!=(const T& scalar) const {
        if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>) {
            return this->operator!=(static_cast<bool>(scalar));
        } else if constexpr (std::is_floating_point_v<std::remove_cv_t<T>>) {
            if constexpr (sizeof(T) <= sizeof(float)) {
                return this->operator!=(static_cast<float>(scalar));
            } else {
                return this->operator!=(static_cast<double>(scalar));
            }
        } else if constexpr (std::is_signed_v<std::remove_cv_t<T>>) {
            if constexpr (sizeof(T) <= sizeof(int8_t)) {
                return this->operator!=(static_cast<int8_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(int16_t)) {
                return this->operator!=(static_cast<int16_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(int32_t)) {
                return this->operator!=(static_cast<int32_t>(scalar));
            } else {
                return this->operator!=(static_cast<int64_t>(scalar));
            }
        } else {
            if constexpr (sizeof(T) <= sizeof(uint8_t)) {
                return this->operator!=(static_cast<uint8_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(uint16_t)) {
                return this->operator!=(static_cast<uint16_t>(scalar));
            } else if constexpr (sizeof(T) <= sizeof(uint32_t)) {
                return this->operator!=(static_cast<uint32_t>(scalar));
            } else {
                return this->operator!=(static_cast<uint64_t>(scalar));
            }
        }
    }

};

# endif
