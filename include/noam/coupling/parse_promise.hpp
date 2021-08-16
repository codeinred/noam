#pragma once
#include <noam/coupling/await_parser.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <tuple>

namespace noam {
template <class T>
struct parse_promise {
    using handle_t = std::coroutine_handle<parse_promise>;
    state_t state;
    T value;
    bool is_good = false;

    constexpr operator bool() const noexcept { return is_good; }
    constexpr state_t get_state() const noexcept { return state; }
    constexpr void set_state(state_t new_state) noexcept { state = new_state; }
    constexpr decltype(auto) get_value() & noexcept { return value; }
    constexpr decltype(auto) get_value() const& noexcept { return value; }
    constexpr decltype(auto) get_value() && noexcept {
        return std::move(*this).value;
    }
    parse_promise() = default;
    parse_promise(parse_promise const&) = default;
    parse_promise(parse_promise&&) = default;
    parse_promise(state_t state) noexcept
      : state(state) {}
    parse_promise(auto&& context, state_t state) noexcept
      : state(state) {}

    constexpr std::suspend_always initial_suspend() noexcept { return {}; }
    constexpr std::suspend_always final_suspend() noexcept { return {}; }

    void return_value(T const& value) {
        this->value = value;
        is_good = true;
    }
    void return_value(T&& value) {
        this->value = std::move(value);
        is_good = true;
    }
    template <class F>
    auto await_transform(F&& func) {
        return await_parser<F> {std::forward<F>(func), &state};
    };

    template <class U>
    auto await_transform(U (*_coro_ptr)()) {
        return await_parser {_coro_ptr(), &state};
    }

    void unhandled_exception() {
        // Fuck this shit we just cancelling it all
        is_good = false;
    }

    /**
     * @brief Obtains the return object.
     *
     * This method returns a coroutine handle to the coroutine associated with
     * this promise object.
     *
     * @return std::coroutine_handle<parse_promise>
     */
    handle_t get_return_object() { return handle_t::from_promise(*this); }
};
} // namespace noam
