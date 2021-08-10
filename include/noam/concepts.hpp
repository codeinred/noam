#pragma once
#include <concepts>
#include <string_view>

namespace noam {
using state_t = std::string_view;

using std::convertible_to;
using std::invocable;
using std::same_as;

// A qualified type is a type that is a reference, is const, or is both
template <class T>
concept qualified_type = std::is_reference_v<T> || std::is_const_v<T>;

/**
 * @brief A concept that matches any result type. Result types must have a
 * good() function, a get_value() function, and a get_state() function.
 *
 * @tparam Result the type to test
 */
template <class Result>
concept parse_result = requires(Result result) {
    { result.good() } -> convertible_to<bool>;
    { result.get_value() };
    { result.get_state() } -> same_as<state_t>;
};

/**
 * @brief A result type that allows resetting the state via set_state. This
 * makes implementation of the lookahead combinator easier for results that
 * support this
 *
 * @tparam Result the type to test
 */
template <class Result>
concept lookahead_enabled_result =
    parse_result<Result> && requires(Result result, state_t state) {
    {result.set_state(state)};
};

/**
 * @brief A concept that matches any parser type. Parsers must have a function
 * parse() which returns a parse_result
 *
 * @tparam Parser the type to test
 */
template <class Parser>
concept any_parser = requires(Parser parser, state_t state) {
    { parser.parse(state) } -> parse_result;
};

template <class Result>
struct result_traits {
    // Individual Result types can certify that good() always returns true
    // but by default it is unknown if result.good() always returns true
    // Used to certify a result type as a Good Boy 🐶
    constexpr static bool always_good = false;
};

// Decay qualified types, so that we can quickly check if those are Good Boys 🐶
template <qualified_type Result>
struct result_traits<Result> : result_traits<std::decay_t<Result>> {
    using base = result_traits<std::decay_t<Result>>;
    using base::always_good;
};

// Indicates that a result is a Good Boy 🐶
template <class Result>
constexpr bool result_always_good_v = result_traits<Result>::always_good;

template <class Result>
constexpr bool result_is_🐶 = result_traits<Result>::always_good;
} // namespace noam
