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
 * good() function, a value() function, and a get_state() function.
 *
 * @tparam Result the type to test
 */
template <class Result>
concept parse_result = requires(Result result) {
    { result.good() } -> convertible_to<bool>;
    { result.value() };
    { result.get_state() } -> same_as<state_t>;
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
} // namespace noam
