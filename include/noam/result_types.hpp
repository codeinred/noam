#pragma once
#include <noam/type_traits.hpp>
#include <noam/util/value_wrappers.hpp>
#include <optional>
#include <utility>

namespace noam {
struct empty {
    bool operator==(empty const&) const = default;
    std::strong_ordering operator<=>(empty const&) const = default;
};

template <class Value>
struct result
  : state_t
  , optional_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::ref_state;
    using state_base::set_state;
    using value_base = optional_result_value<Value>;
    using value_base::get_value;
    using value_base::good;
    using value_base::operator bool;

    result& operator=(result const&) = default;
    result& operator=(result&&) = default;

    /**
     * @brief Provides an assignment operator for Result types other than the
     * current result
     *
     * @tparam Result
     * @param r
     * @return constexpr result&
     */
    template <class Result>
    requires other_than<Result, result&>
    constexpr result& operator=(Result&& r) {
        if constexpr (result_always_good_v<Result>) {
            return operator=(
                result {r.get_state(), std::forward<Result>(r).get_value()});
        } else {
            if (r) {
                return operator=(result {
                    r.get_state(),
                    std::forward<Result>(r).get_value()});
            } else {
                return operator=(result {});
            }
        }
    }
    constexpr bool check_value(Value const& v) const {
        return good() && get_value() == v;
    }
};
template <default_constructible Value>
struct result<Value>
  : state_t
  , basic_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::good;
    using state_base::ref_state;
    using state_base::set_state;
    using state_base::operator bool;
    using value_base = basic_result_value<Value>;
    using value_base::get_value;

    /**
     * @brief Provides an assignment operator for Result types other than the
     * current result
     *
     * @tparam Result
     * @param r
     * @return constexpr result&
     */
    template <class Result>
    requires other_than<Result, result&>
    constexpr result& operator=(Result&& r) {
        if constexpr (result_always_good_v<Result>) {
            return operator=(
                result {r.get_state(), std::forward<Result>(r).get_value()});
        } else {
            if (r) {
                return operator=(result {
                    r.get_state(),
                    std::forward<Result>(r).get_value()});
            } else {
                return operator=(result {});
            }
        }
    }
    constexpr bool check_value(Value const& v) const {
        return good() && get_value() == v;
    }
};
template <class Value>
result(state, Value) -> result<Value>;
template <class Value>
result(state, std::reference_wrapper<Value>) -> result<Value&>;

/**
 * @brief Represents a type that is implicitly convertible to any result<T>,
 * resulting in a default-constructed result<T> representing a failed result.
 *
 */
struct null_result_t {
    template <class T>
    constexpr operator result<T>() const // <br>
        noexcept(noexcept(result<T>())) {
        return result<T>();
    }
};
constexpr null_result_t null_result;

template <class Value>
struct pure_result
  : state_t
  , basic_result_value<Value> {
    using value_type = Value;
    using state_base = state_t;
    using state_base::get_state;
    using state_base::ref_state;
    using state_base::set_state;
    using value_base = basic_result_value<Value>;
    using value_base::get_value;
    constexpr operator bool() const noexcept { return true; }
    constexpr bool good() const noexcept { return true; }
    constexpr operator result<value_type>() const noexcept {
        return {get_state(), get_value()};
    }
    constexpr bool check_value(Value const& v) const {
        return good() && get_value() == v;
    }
};
template <class Value>
pure_result(state_t, Value) -> pure_result<Value>;

using boolean_result = pure_result<bool>;
using state_result = pure_result<state>;
template <class Value>
using optional_result = pure_result<std::optional<Value>>;


// *To be read in the same voice you'd use to speak to a dog who's a good boy.*
// Who's a good boy? pure_result is! You're a good boy, aren't you? Yes you are!
// You're always good and you're idempotent and your state is always identical
// to the one that went in! Good boy, pure_result, good boy!!! ????
template <class Value>
struct result_traits<pure_result<Value>> {
    constexpr static bool always_good = true;
};

// A boolean_result is always good b/c the value returned checks if another
// parser succeeded. This implies boolean_result always returns true.
// ??? it is proveable that boolean_result is also a good boy!
template <>
struct result_traits<boolean_result> {
    constexpr static bool always_good = true;
};

// A state_result is also a Good Boy!!! So many good boys over here ????
template <>
struct result_traits<state_result> {
    constexpr static bool always_good = true;
};

// Optional results are used to transform a non-good result into a good result
// by returning an optional<Value> instead of Value. optional_results are very
// good boys! ????
template <class Value>
struct result_traits<optional_result<Value>> {
    constexpr static bool always_good = true;
};

/**
 * @brief If the result type of Parser is default_constructible, obtains that
 * type, otherwise returns result<parser_value_t<Parser>>, which should always
 * be default constructible
 *
 * @tparam Parser the parser to obtain a result type for
 */
template <class Parser>
using default_constructible_parser_result_t = std::conditional_t<
    std::is_default_constructible_v<parser_result_t<Parser>>,
    parser_result_t<Parser>,
    result<parser_value_t<Parser>>>;

/**
 * @brief If `good` is true, returns `pure_result<Value>`, otherwise returns
 * `result<Value>`
 *
 * @tparam Value
 * @tparam good
 */
template <class Value, bool good>
using get_result_t = std::
    conditional_t<good, pure_result<Value>, result<Value>>;
} // namespace noam
