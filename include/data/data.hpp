# ifndef DATA_HPP
# define DATA_HPP

# include <string>
# include <memory>
# include <cstdint>
# include <type_traits>

/**
 * @brief Abstract interface for payload values attached to Node instances.
 *
 * Implementations provide scalar/string inspection, copy semantics,
 * metadata rendering, and typed scalar comparisons.
 */
class Data {

public:
    /** @brief Virtual destructor for polymorphic use. */
    virtual ~Data() = default;

    /** @brief Create a shallow clone that preserves sharing semantics. */
    virtual std::shared_ptr<Data> clone() const = 0;
    /**
     * @brief Create a copy of this payload.
     * @param deep When true, implementations should duplicate underlying buffers.
     */
    virtual std::shared_ptr<Data> copy(bool deep=false) const = 0;
    
    /** @brief True when payload stores textual content. */
    virtual bool hasString() const = 0;
    /** @brief True when payload is empty/none-like. */
    virtual bool isNone() const = 0;
    /** @brief True when payload is a scalar numeric value. */
    virtual bool isScalar() const = 0;

    /** @brief Extract payload as UTF-8 string when available. */
    virtual std::string extractString() const = 0;

    /** @brief Detailed payload description for debugging/logging. */
    virtual std::string info() const = 0;
    /** @brief Compact payload description. */
    virtual std::string shortInfo() const = 0;

    /** @name Typed scalar comparison overloads
     *  @{
     */
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
    /** @} */

    /**
     * @brief Canonical arithmetic equality adapter.
     * @tparam T Any arithmetic type.
     */
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

    /**
     * @brief Canonical arithmetic inequality adapter.
     * @tparam T Any arithmetic type.
     */
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
