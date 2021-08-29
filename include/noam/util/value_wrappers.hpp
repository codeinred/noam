#pragma once
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
};
template <class Value>
struct optional_result_value<Value&> {
    using value_type = Value&;
    optional_ref<Value> value;
    constexpr Value& get_value() const { return value; }
    constexpr operator bool() const noexcept { return bool(value.pointer); }
    constexpr bool good() const noexcept { return bool(value.pointer); }
};
} // namespace noam
