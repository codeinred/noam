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

class boolean_result {
    state_t state;
    bool value_;

   public:
    boolean_result() = default;
    boolean_result(boolean_result const&) = default;
    boolean_result(boolean_result&&) = default;
    constexpr boolean_result(
        std::string_view initial, parse_result auto&& result) {
        if (result.good()) {
            state = result.new_state();
            value_ = true;
        } else {
            state = initial;
            value_ = false;
        }
    }
    boolean_result& operator=(boolean_result const&) = default;
    boolean_result& operator=(boolean_result&&) = default;
    // It's always good b/c it always has a value
    constexpr bool good() const noexcept { return true; }
    constexpr bool value() const { return value_; }
    constexpr state_t new_state() const { return state; }
};

// used for obtaining the hidden state of the parser (e.g., for lookahead)
// this is not impure since it cannot be used to modify that state
// Rather, state_result is a Good Boy b/c it always produces a value! 🐶
struct state_result {
    state_t state;
    constexpr bool good() const noexcept { return true; }
    constexpr state_t value() const { return state; }
    constexpr state_t new_state() const { return state; }
};

template <class Value>
class standard_result {
   private:
    state_t state;
    Value v;
    bool is_good = false;

   public:
    standard_result() = default;
    standard_result(state_t state, Value value) : state(state), v(value), is_good(true) {}
    standard_result(standard_result const&) = default;
    standard_result(standard_result&&) = default;
    standard_result& operator=(standard_result const&) = default;
    standard_result& operator=(standard_result&&) = default;

    constexpr bool good() const noexcept { return is_good; }
    constexpr state_t new_state() const noexcept { return state; }
    constexpr decltype(auto) value() & { return v; }
    constexpr decltype(auto) value() const& { return v; }
    constexpr decltype(auto) value() && { return std::move(*this).v; }
};
template <class Value>
standard_result(state_t, Value) -> standard_result<Value>;
} // namespace noam
