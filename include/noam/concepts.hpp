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

template <class T>
concept parse_result = requires(T t) {
    { t.good() } -> convertible_to<bool>;
    { t.value() };
    { t.new_state() } -> same_as<state_t>;
};

template <class F>
concept any_parser = requires(F func, state_t state) {
    { func(state) } -> parse_result;
};
} // namespace noam
