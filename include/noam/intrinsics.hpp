#pragma once
#include <charconv>
#include <cstddef>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>

namespace noam {
template <class T>
constexpr parser parse_charconv =
    [](noam::state_t state) -> standard_result<T> {
    auto start_ = state.data();
    auto end_ = state.data() + state.size();
    T value;
    auto result = std::from_chars(start_, end_, value);
    if (result.ec == std::errc()) {
        return {state_t(result.ptr, end_), value};
    } else {
        return {};
    }
} / make_parser;

constexpr parser parse_int = parse_charconv<int>;
constexpr parser parse_uint = parse_charconv<unsigned int>;
constexpr parser parse_int64 = parse_charconv<int64_t>;
constexpr parser parse_uint64 = parse_charconv<uint64_t>;
constexpr parser parse_long = parse_charconv<long>;
constexpr parser parse_ulong = parse_charconv<unsigned long>;
constexpr parser parse_long_long = parse_charconv<long long>;
constexpr parser parse_ulong_long = parse_charconv<unsigned long long>;
constexpr parser parse_float = parse_charconv<float>;
constexpr parser parse_double = parse_charconv<double>;
constexpr parser parse_long_double = parse_charconv<long double>;

/**
 * @brief Matches strings starting with the characters `chars...`. E.g,
 * `parse_constexpr_prefix<'h', 'e', 'l', 'l', 'o'>` matches strings starting
 * with "hello"
 *
 * @tparam chars the prefix to match
 */
template <char... chars>
constexpr parser parse_constexpr_prefix =
    [](state_t state) -> match_constexpr_prefix_result<chars...> {
    int i = 0;
    if (state.size() >= sizeof...(chars) && ((state[i++] == chars) && ...)) {
        state.remove_prefix(sizeof...(chars));
        return match_constexpr_prefix_result<chars...>(state);
    } else {
        return {};
    }
} / make_parser;

template <char... chars>
constexpr parser parse_repeated_char = [](state_t state) {
    size_t size = state.size();
    for (size_t i = 0; i < size; i++) {
        char current = state[i];

        // If one of them matches then we continue testing chars
        if (((chars == current) || ...))
            continue;

        return noam::pure_result {state.substr(i), i};
    }
    return noam::pure_result {state.substr(size), size};
} / make_parser;

/**
 * @brief Parses 0 or more spaces and returns the number of characters read
 *
 */
constexpr parser parse_spaces = parse_repeated_char<' '>;

/**
 * @brief Parses 0 or more tabs and returns the number of characters read
 *
 */
constexpr parser parse_tabs = parse_repeated_char<'\t'>;

/**
 * @brief Parses 0 or more whitespace characters, and returns the number of
 * characters read
 */
constexpr parser whitespace = parse_repeated_char<' ', '\t', '\n', '\r'>;

/**
 * @brief Parses a line. Newline and carriage return characters are not included
 * in the parsed line.
 *
 */
constexpr parser parse_line = [](state_t state) {
    size_t size = state.size();
    for (size_t i = 0; i < size; i++) {
        char current = state[i];

        // If one of them matches then we continue testing chars
        if (current != '\n')
            continue;

        // line_size = i, unless there's a carriage return, in which case
        // line_size = i - 1
        size_t line_size = i;
        if (i > 0 && state[i - 1] == '\r') {
            line_size = i - 1;
        }
        return noam::pure_result {
            state.substr(i + 1), state.substr(0, line_size)};
    }
    return noam::pure_result {state.substr(size), state};
} / make_parser;
} // namespace noam
