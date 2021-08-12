#pragma once
#include <noam/concepts.hpp>
#include <optional>
#include <utility>

namespace noam {
using std::optional;

template <class Value>
struct pure_result {
    state_t state;
    Value v;

    // pure_result is a good boy
    constexpr bool good() const noexcept { return true; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
    constexpr decltype(auto) get_value() & { return v; }
    constexpr decltype(auto) get_value() const& { return v; }
    constexpr decltype(auto) get_value() && { return std::move(*this).v; }
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
    constexpr boolean_result(state_t state, bool value) noexcept
      : state(state)
      , value_(value) {}
    /**
     * @brief Creates a boolean_result such that the initial state is selected
     * if the given result is bad, otherwise result.get_state() is selected. The
     * value corresponds to result.good().
     *
     * @param initial The state to use if result.good() returns false
     * @param result The result to transform into a boolean result
     */
    constexpr boolean_result(
        std::string_view initial, parse_result auto&& result) {
        if (result.good()) {
            state = result.get_state();
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
    constexpr bool get_value() const noexcept { return value_; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
};

// used for obtaining the hidden state of the parser (e.g., for lookahead)
// this is not impure since it cannot be used to modify that state
// Rather, state_result is a Good Boy b/c it always produces a value! 🐶
struct state_result {
    state_t state;
    constexpr bool good() const noexcept { return true; }
    constexpr state_t get_value() const noexcept { return state; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
};

template <class Value>
struct optional_result {
    state_t state;
    std::optional<Value> v {};

   public:
    optional_result(state_t state) noexcept
      : state(state) {}
    optional_result(state_t state, Value value)
      : state(state)
      , v(value) {}
    optional_result(optional_result const&) = default;
    optional_result(optional_result&&) = default;
    optional_result& operator=(optional_result const&) = default;
    optional_result& operator=(optional_result&&) = default;

    constexpr bool good() const noexcept { return true; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
    constexpr decltype(auto) get_value() & noexcept { return v; }
    constexpr decltype(auto) get_value() const& noexcept { return v; }
    constexpr decltype(auto) get_value() && noexcept {
        return std::move(*this).v;
    }
};

template <class Value>
class standard_result {
   private:
    state_t state;
    Value v;
    bool is_good = false;

   public:
    standard_result() = default;
    standard_result(state_t state, Value value)
      : state(state)
      , v(value)
      , is_good(true) {}
    standard_result(standard_result const&) = default;
    standard_result(standard_result&&) = default;
    standard_result& operator=(standard_result const&) = default;
    standard_result& operator=(standard_result&&) = default;

    constexpr bool good() const noexcept { return is_good; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
    constexpr decltype(auto) get_value() & noexcept { return v; }
    constexpr decltype(auto) get_value() const& noexcept { return v; }
    constexpr decltype(auto) get_value() && noexcept {
        return std::move(*this).v;
    }
};
template <class Value>
standard_result(state_t, Value) -> standard_result<Value>;

template <char... ch>
struct match_constexpr_prefix_result {
   private:
    constexpr static char ch_array[sizeof...(ch) + 1] {ch..., '\0'};
    /**
     * @brief The prefix that this result matches
     *
     */
    constexpr static std::string_view value {ch_array, sizeof...(ch)};
    state_t state;
    bool is_good = false;

   public:
    match_constexpr_prefix_result() = default;
    match_constexpr_prefix_result(state_t state) noexcept
      : state(state)
      , is_good(true) {}
    constexpr bool good() const noexcept { return is_good; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
    constexpr std::string_view get_value() const noexcept { return value; }
};

// The result of fmap'ing a parser result. It preserves BaseResult.good() and
// BaseResult.get_state(), however transform_result.get_value() is given by
// func(BaseResult.get_value())
template <class BaseResult, class Func>
struct transform_result : BaseResult {
    [[no_unique_address]] Func func;
    using BaseResult::good;
    using BaseResult::new_state;
    constexpr decltype(auto)
    get_value() & noexcept(noexcept(func(BaseResult::get_value()))) {
        return func(BaseResult::get_value());
    }
    constexpr decltype(auto)
    get_value() const& noexcept(noexcept(func(BaseResult::get_value()))) {
        return func(BaseResult::get_value());
    }
    constexpr decltype(auto) get_value() && noexcept(noexcept(
        std::move(*this).func(std::move(*this).BaseResult::get_value()))) {
        return std::move(*this).func(std::move(*this).BaseResult::get_value());
    }
};
template <class BaseResult, class Func>
transform_result(BaseResult, Func) -> transform_result<BaseResult, Func>;

// *To be read in the same voice you'd use to speak to a dog who's a good boy.*
// Who's a good boy? pure_result is! You're a good boy, aren't you? Yes you are!
// You're always good and you're idempotent and your state is always identical
// to the one that went in! Good boy, pure_result, good boy!!! 🐶
template <class Value>
struct result_traits<pure_result<Value>> {
    constexpr static bool always_good = true;
};

// A boolean_result is always good b/c the value returned checks if another
// parser succeeded. This implies boolean_result.good() always returns true.
// ∴ it is proveable that boolean_result is also a good boy!
template <>
struct result_traits<boolean_result> {
    constexpr static bool always_good = true;
};

// A state_result is also a Good Boy!!! So many good boys over here 🐶
template <>
struct result_traits<state_result> {
    constexpr static bool always_good = true;
};

// Optional results are used to transform a non-good result into a good result
// by returning an optional<Value> instead of Value. optional_results are very
// good boys! 🐶
template <class Value>
struct result_traits<optional_result<Value>> {
    constexpr static bool always_good = true;
};

// transform_result<BaseResult, Func> is a good boy if and only if BaseResult
// is a good boy. 🐶
template <class BaseResult, class Func>
struct result_traits<transform_result<BaseResult, Func>> {
    constexpr static bool always_good = result_always_good_v<BaseResult>;
};
} // namespace noam
