#pragma once
#include <noam/util/value_wrappers.hpp>
#include <noam/concepts.hpp>
#include <optional>
#include <utility>

namespace noam {
struct empty {};

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
    constexpr operator result<value_type>() const noexcept {
        return {get_state(), get_value()};
    }
};
template <class Value>
pure_result(state_t, Value) -> pure_result<Value>;

using boolean_result = pure_result<bool>;
using state_result = pure_result<state>;
template <class Value>
using optional_result = pure_result<std::optional<Value>>;

template <char... ch>
struct match_constexpr_prefix_result : state {
   private:
    constexpr static char ch_array[sizeof...(ch) + 1] {ch..., '\0'};

   public:
    /**
     * @brief The prefix that this result matches
     *
     */
    constexpr static state value {ch_array, sizeof...(ch)};
    using value_type = state;
    match_constexpr_prefix_result() = default;
    match_constexpr_prefix_result(state st) noexcept
      : state(st) {}
    match_constexpr_prefix_result(match_constexpr_prefix_result const&) =
        default;
    match_constexpr_prefix_result(match_constexpr_prefix_result&&) = default;
    match_constexpr_prefix_result&
    operator=(match_constexpr_prefix_result const&) = default;
    match_constexpr_prefix_result&
    operator=(match_constexpr_prefix_result&&) = default;
    using state::get_state;
    using state::operator bool;
    using state::set_state;
    constexpr state_t get_value() const noexcept { return value; }
    constexpr operator result<value_type>() const noexcept {
        return {get_state(), get_value()};
    }
};

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
} // namespace noam
