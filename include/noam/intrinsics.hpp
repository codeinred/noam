#pragma once
#include <charconv>
#include <cstddef>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>

namespace noam {
template <class T>
constexpr parser parse_charconv = [](state_t state) -> result<T> {
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

constexpr parser parse_short = parse_charconv<short>;
constexpr parser parse_ushort = parse_charconv<unsigned short>;
constexpr parser parse_int = parse_charconv<int>;
constexpr parser parse_uint = parse_charconv<unsigned int>;
constexpr parser parse_long = parse_charconv<long>;
constexpr parser parse_ulong = parse_charconv<unsigned long>;
constexpr parser parse_long_long = parse_charconv<long long>;
constexpr parser parse_ulong_long = parse_charconv<unsigned long long>;

constexpr parser parse_int8 = parse_charconv<int8_t>;
constexpr parser parse_uint8 = parse_charconv<uint8_t>;
constexpr parser parse_int16 = parse_charconv<int16_t>;
constexpr parser parse_uint16 = parse_charconv<uint16_t>;
constexpr parser parse_int32 = parse_charconv<int32_t>;
constexpr parser parse_uint32 = parse_charconv<uint32_t>;
constexpr parser parse_int64 = parse_charconv<int64_t>;
constexpr parser parse_uint64 = parse_charconv<uint64_t>;

constexpr parser parse_float = parse_charconv<float>;
constexpr parser parse_double = parse_charconv<double>;
constexpr parser parse_long_double = parse_charconv<long double>;

/**
 * @brief Matches one character in `{ char_set... }`
 *
 * @tparam char_set The set of characters that can be matched
 */
template <char... char_set>
constexpr parser match_ch = {[](state_t state) -> result<empty> {
    if (state._begin < state._end) {
        char first = *state._begin;
        if (((char_set == first) || ...))
            return {state.substr(1), empty {}};
    }
    return {};
}};
/**
 * @brief Matches zero or more characters in `{ char_set... }`
 *
 * @tparam char_set The set of characters that can be matched
 */
template <char... char_set>
constexpr parser match_any_ch = {[](state_t state) -> pure_result<empty> {
    char const* begin = state._begin;
    char const* end = state._end;
    while (begin < end && ((*begin == char_set) || ...))
        begin++;
    return {state_t {begin, end}, empty {}};
}};

/**
 * @brief Matches one space character (`' '`)
 *
 */
constexpr parser match_space = match_ch<' '>;
/**
 * @brief Matches zero or more space characters
 *
 */
constexpr parser match_spaces = match_any_ch<' '>;

template <char... prefix>
constexpr parser match_constexpr_prefix = {[](state_t state) -> result<empty> {
    if (sizeof...(prefix) <= state.size()) {
        char const* begin = state._begin;
        const char* end = state._end;
        if (((*begin++ == prefix) && ...)) {
            return {state_t {begin, end}, empty {}};
        }
    }
    return {};
}};

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
constexpr parser count_ch = [](state_t state) {
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
constexpr parser count_spaces = count_ch<' '>;

/**
 * @brief Parses 0 or more tabs and returns the number of characters read
 *
 */
constexpr parser count_tabs = count_ch<'\t'>;

/**
 * @brief Matches 0 or more whitespace characters
 */
constexpr parser whitespace = match_any_ch<' ', '\t', '\n', '\r'>;

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

template <char... separator>
constexpr parser match_separator =
    match_spaces >> match_constexpr_prefix<separator...> >> match_spaces;

constexpr parser match_comma_separator = match_separator<','>;
} // namespace noam
