#pragma once
#include <concepts>
#include <noam/state.hpp>
namespace noam {

using std::convertible_to;
using std::invocable;
using std::same_as;

// A qualified type is a type that is a reference, is const, or is both
template <class T>
concept qualified_type = std::is_reference_v<T> || std::is_const_v<T>;

/**
 * @brief Represents any type other than *Except*
 *
 * @tparam T the type to test
 * @tparam Except the types to exclude
 */
template <class T, class... Except>
concept other_than = !(std::is_same_v<T, Except> || ...);

/**
 * @brief A concept that matches any result type. Result types must have a
 * good() function, a get_value() function, and a get_state() function.
 *
 * @tparam Result the type to test
 */
template <class Result>
concept parse_result = requires(Result result) {
    { result } -> convertible_to<bool>;
    { result.get_value() };
    { result.get_state() } -> same_as<state_t>;
};

template <class T>
concept default_constructible = std::is_default_constructible_v<T>;

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
    // but by default it is unknown if result always returns true
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

/**
 * @brief Obtains the value type produced by the result
 *
 * @tparam Result the result type
 */
template <class Result>
using result_value_t = typename Result::value_type;

/**
 * @brief Returns the result type produced by a parser, aka
 * decltype(Parser.parse(...))
 *
 * @tparam Parser
 */
template <class Parser>
using parser_result_t = std::decay_t<decltype(std::declval<Parser>().parse(
    std::declval<state_t>()))>;

/**
 * @brief Checks if a parser returns a result that is always good
 *
 * @tparam Parser
 */
template <class Parser>
constexpr bool parser_always_good_v =
    result_always_good_v<parser_result_t<Parser>>;

/**
 * @brief Returns the value type of the result returned by a parser
 *
 * @tparam Parser the parser
 */
template <class Parser>
using parser_value_t = result_value_t<parser_result_t<Parser>>;

/**
 * @brief Represents any result type that is always good. This guarantee allows
 * certain optimizations to take place with regard to the implementation of
 * things like noam::await_parser.
 *
 * @tparam Result the type to test
 */
template <class Result>
concept always_good_result =
    parse_result<Result> && result_always_good_v<Result>;

/**
 * @brief Represents a parser that produces an always_good_result.
 *
 * @tparam Parser the type to test
 */
template <class Parser>
concept always_good_parser =
    any_parser<Parser> && requires(Parser parser, state_t state) {
    { parser.parse(state) } -> always_good_result;
};
} // namespace noam
