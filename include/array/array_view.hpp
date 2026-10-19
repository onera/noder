#ifndef NODER_ARRAY_ARRAY_VIEW_HPP
#define NODER_ARRAY_ARRAY_VIEW_HPP

#include <cstddef>
#include <type_traits>
#include <vector>

/**
 * @brief Non-owning view over a contiguous sequence.
 *
 * This provides the pointer-and-size operations needed by Noder and its
 * clients without depending on the C++20 <span> header.
 */
template <typename T>
class ArrayView
{
public:
    using element_type = T;
    using value_type = typename std::remove_cv<T>::type;
    using size_type = std::size_t;
    using pointer = T*;
    using reference = T&;
    using iterator = T*;

    constexpr ArrayView() noexcept = default;

    constexpr ArrayView(pointer data, size_type size) noexcept
        : data_(data), size_(size)
    {
    }

    template <typename U,
              typename Allocator,
              typename std::enable_if<
                  std::is_convertible<U*, T*>::value,
                  int>::type = 0>
    constexpr ArrayView(std::vector<U, Allocator>& values) noexcept
        : data_(values.data()), size_(values.size())
    {
    }

    template <typename U,
              typename Allocator,
              typename std::enable_if<
                  std::is_convertible<const U*, T*>::value,
                  int>::type = 0>
    constexpr ArrayView(const std::vector<U, Allocator>& values) noexcept
        : data_(values.data()), size_(values.size())
    {
    }

    template <typename U,
              typename std::enable_if<
                  std::is_convertible<U*, T*>::value,
                  int>::type = 0>
    constexpr ArrayView(const ArrayView<U>& other) noexcept
        : data_(other.data()), size_(other.size())
    {
    }

    constexpr pointer data() const noexcept { return data_; }
    constexpr size_type size() const noexcept { return size_; }
    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr iterator begin() const noexcept { return data_; }
    constexpr iterator end() const noexcept
    {
        return size_ == 0 ? data_ : data_ + size_;
    }

    constexpr reference operator[](size_type index) const noexcept
    {
        return data_[index];
    }

private:
    pointer data_ = nullptr;
    size_type size_ = 0;
};

#endif
