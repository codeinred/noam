#pragma once
#include <concepts>
#include <noam/type_traits.hpp>
#include <optional>

namespace noam {
/**
 * @brief Represents either a reference or a null ptr
 *
 * @tparam Value the value to point to
 */
template <class Value>
struct optional_ref {
    Value* pointer = nullptr;
    optional_ref() = default;
    constexpr optional_ref(Value& value) noexcept
      : pointer(&value) {}
    optional_ref(optional_ref const&) = default;
    optional_ref(optional_ref&&) = default;
    optional_ref& operator=(optional_ref const&) = default;
    optional_ref& operator=(optional_ref&&) = default;
    constexpr operator Value&() const noexcept { return *pointer; }
};
template <class Value>
optional_ref(Value&) -> optional_ref<Value>;
template <class Value>
optional_ref(std::reference_wrapper<Value>) -> optional_ref<Value>;

using std::optional;
template <class Value>
struct basic_result_value {
    using value_type = Value;
    [[no_unique_address]] Value value = Value();
    constexpr decltype(auto) get_value() & { return (value); }
    constexpr decltype(auto) get_value() const& { return (value); }
    constexpr decltype(auto) get_value() && { return (std::move(*this).value); }
};
template <class Value>
struct optional_result_value {
    using value_type = Value;
    std::optional<Value> value;
    constexpr decltype(auto) get_value() & { return (*value); }
    constexpr decltype(auto) get_value() const& { return (*value); }
    constexpr decltype(auto) get_value() && { return (std::move(*value)); }
    constexpr operator bool() const noexcept { return bool(value); }
    constexpr bool good() const noexcept { return bool(value); }

    optional_result_value& operator=(optional_result_value const&) = default;
    optional_result_value& operator=(optional_result_value&&) = default;
    constexpr optional_result_value& operator=(Value const& v) {
        value = v;
        return *this;
    }
    constexpr optional_result_value& operator=(Value&& v) {
        value = std::move(v);
        return *this;
    }
};
template <class Value>
struct optional_result_value<Value&> {
    using value_type = Value&;
    optional_ref<Value> value;
    constexpr Value& get_value() const { return value; }
    constexpr operator bool() const noexcept { return bool(value.pointer); }
    constexpr bool good() const noexcept { return bool(value.pointer); }

    optional_result_value& operator=(optional_result_value const&) = default;
    optional_result_value& operator=(optional_result_value&&) = default;
    constexpr optional_result_value& operator=(Value& ref) noexcept {
        value = optional_ref(ref);
        return *this;
    }
};

template <class T>
struct box : std::optional<T> {
    using base = std::optional<T>;
    using base::base;
    using base::operator=;
    using base::operator bool;
    using base::value;
    box() = default;
    box(box const&) = default;
    box(box&&) = default;
    constexpr box& operator=(box const&) = default;
    constexpr box& operator=(box&&) = default;

    constexpr operator T&() & { return value(); }
    constexpr operator T&&() && { return std::move(*this).value(); }
    constexpr operator const T&() const& { return value(); }
};

template <class T>
struct box<T&> {
    T* pointer = nullptr;
    using value_type = T&;

    // Boilerplate / defaults
    box() = default;
    box(box const&) = default;
    box(box&&) = default;
    constexpr box& operator=(box const&) = default;
    constexpr box& operator=(box&&) = default;

    constexpr box(T& ref) noexcept
      : pointer(&ref) {}
    constexpr box& operator=(T& ref) noexcept {
        pointer = &ref;
        return *this;
    }
    constexpr operator T&() const noexcept { return *pointer; }
};

template <default_constructible T>
struct box<T> {
    T value;
    using value_type = T;

    // Boilerplate / defaults
    // If we create it as an aggregate I don't believe the constructors are
    // needed

    // box() = default; box(box const&) = default; box(box&&) = default;
    // box(T const& t) noexcept(std::is_nothrow_copy_constructible_v<T>)
    //   : value(t) {}
    // box(T&& t) noexcept(std::is_nothrow_move_constructible_v<T>)
    //   : value(std::move(t)) {}
    constexpr box& operator=(box const&) = default;
    constexpr box& operator=(box&&) = default;

    constexpr operator T&() & noexcept { return value; }
    constexpr operator T&&() && noexcept { return std::move(value); }
    constexpr operator const T&() const& noexcept { return value; }
    template <class U>
    constexpr box& operator=(U&& u) // <br>
        noexcept(noexcept(value = std::forward<U>(u))) {
        value = std::forward<U>(u);
        return *this;
    }
};

/**
 * @brief Places non-default-constructible types in a box that can be
 * default-constructed. This simplifies alggorithms such as either.
 *
 * @tparam T the type to box.
 */
template <class T>
using box_if_necessary_t = std::
    conditional_t<std::is_default_constructible_v<T>, T, box<T>>;
} // namespace noam
