#pragma once
#include <noam/concepts.hpp>

namespace noam {
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

template <class Value>
struct pure_result {
    state_t state;
    Value v;

    // pure_result is a good boy
    constexpr bool good() const noexcept { return true; }
    constexpr state_t new_state() const noexcept { return state; }
    constexpr decltype(auto) value() & { return v; }
    constexpr decltype(auto) value() const& { return v; }
    constexpr decltype(auto) value() && { return std::move(*this).v; }
};
template <class Value>
pure_result(state_t, Value) -> pure_result<Value>;
} // namespace noam
