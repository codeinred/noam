#pragma once
#include <charconv>
#include <cstddef>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/combinator_types.hpp>
#include <string>
#include <noam/util/intrinsic_types.hpp>

namespace noam {
template <class T>
constexpr parser parse_charconv = parser { parsers::charconv<T>{} };

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

template <any_literal... lit>
constexpr parser literal = parser {parsers::literal<lit...>()};
template <class T, any_literal... lit>
constexpr parser literal_makes = parser {parsers::literal_makes<T, lit...>()};
template <auto constant, any_literal... lit>
constexpr parser literal_constant =
    parser {parsers::literal_constant<constant, lit...>()};

/**
 * @brief Matches one character in `{ char_set... }`
 *
 * @tparam char_set The set of characters that can be matched
 */
template <char... char_set>
constexpr parser match_ch = literal<char_set...>;

/**
 * @brief Matches zero or more characters the sequnce chars...
 *
 * @tparam chars the sequence of characters to match against
 */
template <char... chars>
constexpr parser match_chs = { parsers::zero_or_more_chars<chars...>() };

/**
 * @brief Matches one space character (`' '`)
 *
 */
constexpr parser match_space = match_ch<' '>;
/**
 * @brief Matches zero or more space characters
 *
 */
constexpr parser match_spaces = match_chs<' '>;

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
constexpr parser whitespace = match_chs<' ', '\t', '\n', '\r'>;

/**
 * @brief Matches 0 or more whitespace characters. Shorthand for
 * noam::whitespace
 */
constexpr parser ws = whitespace;

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

/**
 * @brief Matches a particular separator character surrounded by whitespace
 *
 * @tparam sep
 */
template <char sep>
constexpr parser separator = whitespace >> match_ch<sep> >> whitespace;

constexpr parser comma_separator = separator<','>;

constexpr parser parse_bool {[](state_t st) -> result<bool> {
    if (st.starts_with("true")) {
        return {st.substr(4), true};
    }
    if (st.starts_with("false")) {
        return {st.substr(5), false};
    }
    return {};
}};

constexpr parser parse_string = parser {[](state_t st) -> result<std::string> {
    if (st.size() >= 2 && st[0] == '"') {
        std::string str;
        size_t i = 1;
        size_t count = st.size() - 1;
        while (i < count) {
            char c = st[i];
            char val = c;
            if (c == '\\') {
                char next = st[i + 1];
                switch (next) {
                    case 'b': val = '\b'; break;
                    case 'f': val = '\f'; break;
                    case 'n': val = '\n'; break;
                    case 'r': val = '\r'; break;
                    case 't': val = '\t'; break;
                    case '\\': val = '\\'; break;
                    case '"': val = '"'; break;
                    // The parser fails on an unrecognized escape code
                    default: return {};
                }
                i += 2;
            } else if (c == '"') {
                break;
            } else {
                val = c;
                i += 1;
            }
            str.push_back(val);
        }
        if (st[i] == '"') {
            return {st.substr(i + 1), std::move(str)};
        }
    }
    return {};
}};

/**
 * @brief Parses a section of characters between an opening and closing quote.
 * The section of characters is returned as a string_view. The string itself is
 * unmodified: any escaped characters in the string remain escaped.
 *
 */
constexpr parser parse_string_view {[](state_t st) -> result<std::string_view> {
    if (st.size() >= 2 && st[0] == '"') {
        for (intptr_t i = 1; i < st.size(); i++) {
            if (st[i] == '"' && st[i - 1] != '\\') {
                return {st.substr(i + 1), st.substr(1, i - 1)};
            }
        }
    }
    return {};
}};

static_assert(
    parse_string_view.parse(R"("")").check_value(""),
    "parse_string_view broken");
static_assert(
    parse_string_view.parse(R"("hello")").check_value("hello"),
    "parse_string_view broken");
static_assert(
    parse_string_view.parse(R"("hello" world)").check_value("hello"),
    "parse_string_view broken");
static_assert(
    parse_string_view.parse(R"("hello\" world")")
        .check_value(R"(hello\" world)"),
    "parse_string_view broken");

} // namespace noam
