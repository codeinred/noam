#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace noam {
struct state {
    constexpr static const char* get_end_of_c_string(char const* c_string) {
        while (*c_string)
            c_string++;
        return c_string;
    }
    char const* _begin = nullptr;
    char const* _end = nullptr;

    constexpr state(std::nullptr_t) noexcept {}
    constexpr state() = default;
    constexpr state(state const&) = default;
    constexpr state(state&&) = default;
    state(std::string& str) noexcept
      : _begin(str.data())
      , _end(str.data() + str.size()) {}
    state(std::string const& str) noexcept
      : _begin(str.data())
      , _end(str.data() + str.size()) {}
    state(std::string&& str) noexcept = delete;
    constexpr state(std::string_view sv) noexcept
      : _begin(sv.data())
      , _end(sv.data() + sv.size()) {}
    constexpr state(char const* begin_, char const* end_) noexcept
      : _begin(begin_)
      , _end(end_) {}
    constexpr state(char const* begin, size_t size) noexcept
      : _begin(begin)
      , _end(begin + size) {}
    constexpr state(char const* begin, int64_t size) noexcept
      : _begin(begin)
      , _end(begin + size) {}

    template <size_t N>
    constexpr state(char const (&array)[N]) noexcept
      : _begin(array)
      , _end(array + N - (array[N - 1] == '\0')) {}

    constexpr state(const char* c_ctr) noexcept
      : _begin(c_ctr)
      , _end(get_end_of_c_string(c_ctr)) {}
    constexpr state substr(size_t start) const noexcept {
        return {_begin + start, _end};
    }
    constexpr state substr(size_t start, size_t count) const noexcept {
        size_t max = size() - start;
        if (max < count)
            return state(_begin + start, max);
        else
            return state(_begin + start, count);
    }

    state& operator=(state const&) = default;
    state& operator=(state&&) = default;
    constexpr char const* begin() const noexcept { return _begin; }
    constexpr char const* end() const noexcept { return _end; }
    constexpr char const* data() const noexcept { return _begin; }
    constexpr char const& operator[](size_t index) const noexcept {
        return _begin[index];
    }
    constexpr char const& first() const noexcept { return *_begin; }
    constexpr char const& last() const noexcept { return *(_end - 1); }
    /**
     * @brief Checks if the state refers to a valid view (_begin != nullptr)
     *
     * @return true if _begin != nullptr
     * @return false if _begin == nullptr
     */
    constexpr bool good() const noexcept { return _begin; }
    /**
     * @brief Checks if the state refers to a valid view (_begin != nullptr)
     *
     * @return true if _begin != nullptr
     * @return false if _begin == nullptr
     */
    constexpr operator bool() const noexcept { return _begin; }
    /**
     * @brief Checks if the state is null
     *
     * @return true if _begin == nullptr
     * @return false if _begin != nullptr
     */
    constexpr bool null() const noexcept { return !_begin; }

    /**
     * @brief Returns true if size() == 0 or if the state is null
     *
     * @return true if the state is empty or null
     * @return false if the state points to a range of chars with nonzero length
     */
    constexpr bool empty() const noexcept { return _end <= _begin; }
    constexpr bool has() const noexcept { return _begin < _end; }

    /**
     * @brief Returns the number of characters. This is given by _end - _begin.
     * Allows negatives so that size() > 0 is a reliable check, and you can
     * detect if it overshot it's bounds. If the state is null, returns 0.
     *
     * @return intptr_t The result of _end - _begin
     */
    constexpr intptr_t size() const noexcept { return _end - _begin; }
    /**
     * @brief Returns the number of characters. This is given by _end - _begin.
     * Allows negatives so that size() > 0 is a reliable check, and you can
     * detect if it overshot it's bounds
     *
     * @return intptr_t The result of _end - _begin
     */
    constexpr intptr_t length() const noexcept { return _end - _begin; }
    constexpr void remove_prefix(size_t n) noexcept { _begin += n; }
    constexpr void remove_suffix(size_t n) noexcept { _end -= n; }
    constexpr void swap(state& other) noexcept {
        std::swap(_begin, other._begin);
        std::swap(_end, other._end);
    }
    constexpr bool starts_with(state prefix) const noexcept {
        auto prefix_len = prefix.size();
        if (size() < prefix_len) {
            return false;
        }
        for (intptr_t i = 0; i < prefix_len; i++) {
            if (prefix._begin[i] != _begin[i]) {
                return false;
            }
        }
        return true;
    }
    template <size_t N>
    constexpr bool starts_with(char const (&prefix)[N]) const noexcept {
        if (size() < N) {
            return false;
        }
        for (size_t i = 0; i < N; i++) {
            if (prefix[i] != _begin[i]) {
                return false;
            }
        }
        return true;
    }
    /**
     * @brief Makes the state null
     *
     */
    constexpr void clear() noexcept {
        _begin = nullptr;
        _end = nullptr;
    }
    constexpr bool starts_with(char const ch) const noexcept {
        return _begin < _end && _begin[0] == ch;
    }
    constexpr operator std::string_view() const noexcept {
        return std::string_view(_begin, _end);
    }
    operator std::string() const noexcept { return std::string(_begin, _end); }

    constexpr bool operator==(state other) const noexcept {
        const auto other_length = other.size();
        if (other_length != size())
            return false;
        for (intptr_t i = 0; i < other_length; i++) {
            if (other._begin[i] != _begin[i])
                return false;
        }
        return true;
    }
};
using state_t = state;
} // namespace noam
