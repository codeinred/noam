#pragma once
#include <noam/concepts.hpp>
#include <noam/coupling/await_parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <optional>
#include <string_view>

namespace noam {
template <class T>
struct parse_promise {
    using handle_t = std::coroutine_handle<parse_promise>;
    state_t current_state;
    T value;
    bool is_good = false;
    void set_initial_state(state_t state) { current_state = state; }

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
    template <any_parser F>
    auto await_transform(F&& func) {
        return await_parser<std::decay_t<F>> {
            std::forward<F>(func), &current_state};
    };

    template <class U>
    auto await_transform(U (*_coro_ptr)()) {
        return await_parser {_coro_ptr(), &current_state};
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

    standard_result<T> get_parse_result() && {
        if (is_good) {
            return standard_result<T>(current_state, std::move(value));
        } else {
            return standard_result<T>();
        }
    }
};
template <class T>
class co_parse {
    using handle_t = std::coroutine_handle<parse_promise<T>>;
    handle_t handle_;

   public:
    using promise_type = parse_promise<T>;
    co_parse() = default;
    co_parse(co_parse&& d) noexcept
      : handle_(std::exchange(d.handle_, nullptr)) {}
    co_parse(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

    // This should only ever be run once, so it must be run either
    // on a prvalue co_parse or a moved co_parse
    standard_result<T> operator()(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise()).get_parse_result();
    }
    standard_result<T> parse(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise()).get_parse_result();
    }
    ~co_parse() { handle_.destroy(); }
};
} // namespace noam
