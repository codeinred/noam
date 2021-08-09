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
} // namespace noam
