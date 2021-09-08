#pragma once
#include <charconv>
#include <cstddef>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/combinator_types.hpp>
#include <noam/util/intrinsic_types.hpp>
#include <string>

namespace noam {
template <class T>
constexpr parser parse_charconv {parsers::charconv<T> {}};

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
constexpr parser literal {parsers::literal<lit...> {}};
template <class T, any_literal... lit>
constexpr parser literal_makes {parsers::literal_makes<T, lit...> {}};
template <auto constant, any_literal... lit>
constexpr parser literal_constant {
    parsers::literal_constant<constant, lit...> {}};

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
constexpr parser match_chs {parsers::zero_or_more_chars<chars...> {}};

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
constexpr parser count_ch {parsers::count_chars<chars...> {}};

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
constexpr parser separator {
    parsers::match {whitespace, match_ch<sep>, whitespace}};

constexpr parser comma_separator = separator<','>;

constexpr parser parse_bool {parsers::bool_parser {}};

constexpr parser parse_string {parsers::string_parser {}};

/**
 * @brief Parses a section of characters between an opening and closing quote.
 * The section of characters is returned as a string_view. The string itself is
 * unmodified: any escaped characters in the string remain escaped.
 *
 */
constexpr parser parse_string_view {parsers::view_parser<'"', '"', '\\'> {}};

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
