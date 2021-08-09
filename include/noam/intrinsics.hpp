#pragma once
#include <charconv>
#include <cstddef>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/then_operator.hpp>

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
} // namespace noam
