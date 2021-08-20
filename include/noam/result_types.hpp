#pragma once
#include <noam/concepts.hpp>
#include <optional>
#include <utility>

namespace noam {
struct empty {};

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
    constexpr decltype(auto) get_value() & { return value; }
    constexpr decltype(auto) get_value() const& { return value; }
    constexpr decltype(auto) get_value() && { return std::move(*this).value; }
};
template <class Value>
struct optional_result_value {
    using value_type = Value;
    std::optional<Value> value;
    constexpr decltype(auto) get_value() & { return *value; }
    constexpr decltype(auto) get_value() const& { return *value; }
    constexpr decltype(auto) get_value() && { return std::move(value); }
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

template <class Value>
struct pure_result
  : state_t
  , basic_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::set_state;
    using value_base = basic_result_value<Value>;
    using value_base::get_value;
    constexpr operator bool() const noexcept { return true; }
    constexpr bool good() const noexcept { return true; }
};
template <class Value>
pure_result(state_t, Value) -> pure_result<Value>;

using boolean_result = pure_result<bool>;
using state_result = pure_result<state>;
template <class Value>
using optional_result = pure_result<std::optional<Value>>;

template <class Value>
struct result
  : state_t
  , optional_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::set_state;
    using value_base = optional_result_value<Value>;
    using value_base::get_value;
    using value_base::good;
    using value_base::operator bool;
};
template <default_constructible Value>
struct result<Value>
  : state_t
  , basic_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::good;
    using state_base::set_state;
    using state_base::operator bool;
    using value_base = basic_result_value<Value>;
    using value_base::get_value;
};
template <class Value>
result(state, Value) -> result<Value>;
template <class Value>
result(state, std::reference_wrapper<Value>) -> result<Value&>;

template <char... ch>
struct match_constexpr_prefix_result : state_t {
   private:
    constexpr static char ch_array[sizeof...(ch) + 1] {ch..., '\0'};

   public:
    /**
     * @brief The prefix that this result matches
     *
     */
    constexpr static state_t value {ch_array, sizeof...(ch)};
    using value_type = state_t;
    match_constexpr_prefix_result() = default;
    match_constexpr_prefix_result(state_t state) noexcept
      : state_t(state) {}
    using state::get_state;
    using state::operator bool;
    constexpr void set_state(state_t new_state) noexcept {
        state::operator=(new_state);
    }
    constexpr state_t get_value() const noexcept { return value; }
    constexpr operator result<value_type>() const noexcept {
        return {get_state(), get_value()};
    }
};

// The result of fmap'ing a parser result. It preserves BaseResult and
// BaseResult.get_state(), however transform_result.get_value() is given by
// func(BaseResult.get_value())
template <class BaseResult, class Func>
struct transform_result : BaseResult {
    using value_type = std::invoke_result_t<Func, result_value_t<BaseResult>>;
    [[no_unique_address]] Func func;
    using BaseResult::operator bool;
    using BaseResult::get_state;
    constexpr decltype(auto)
    get_value() & noexcept(noexcept(func(BaseResult::get_value()))) {
        return func(BaseResult::get_value());
    }
    constexpr decltype(auto)
    get_value() const& noexcept(noexcept(func(BaseResult::get_value()))) {
        return func(BaseResult::get_value());
    }
    constexpr decltype(auto) get_value() && noexcept(noexcept(
        std::move(*this).func(std::move(*this).BaseResult::get_value()))) {
        return std::move(*this).func(std::move(*this).BaseResult::get_value());
    }
    constexpr operator result<value_type>() & noexcept(noexcept(get_value())) {
        if (*this)
            return {get_state(), func(BaseResult::get_value())};
        else
            return {};
    }
    constexpr
    operator result<value_type>() const& noexcept(noexcept(get_value())) {
        if (*this)
            return {get_state(), func(BaseResult::get_value())};
        else
            return {};
    }
    constexpr operator result<value_type>() && noexcept(
        noexcept(std::move(*this).get_value())) {
        if (*this)
            return {
                get_state(),
                std::move(*this).func(
                    std::move(*this).BaseResult::get_value())};
        else
            return {};
    }
};
template <class BaseResult, class Func>
transform_result(BaseResult, Func) -> transform_result<BaseResult, Func>;

// *To be read in the same voice you'd use to speak to a dog who's a good boy.*
// Who's a good boy? pure_result is! You're a good boy, aren't you? Yes you are!
// You're always good and you're idempotent and your state is always identical
// to the one that went in! Good boy, pure_result, good boy!!! 🐶
template <class Value>
struct result_traits<pure_result<Value>> {
    constexpr static bool always_good = true;
};

// A boolean_result is always good b/c the value returned checks if another
// parser succeeded. This implies boolean_result always returns true.
// ∴ it is proveable that boolean_result is also a good boy!
template <>
struct result_traits<boolean_result> {
    constexpr static bool always_good = true;
};

// A state_result is also a Good Boy!!! So many good boys over here 🐶
template <>
struct result_traits<state_result> {
    constexpr static bool always_good = true;
};

// Optional results are used to transform a non-good result into a good result
// by returning an optional<Value> instead of Value. optional_results are very
// good boys! 🐶
template <class Value>
struct result_traits<optional_result<Value>> {
    constexpr static bool always_good = true;
};

// transform_result<BaseResult, Func> is a good boy if and only if BaseResult
// is a good boy. 🐶
template <class BaseResult, class Func>
struct result_traits<transform_result<BaseResult, Func>> {
    constexpr static bool always_good = result_always_good_v<BaseResult>;
};
} // namespace noam
