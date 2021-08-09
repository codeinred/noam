#pragma once
#include <noam/concepts.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <optional>
#include <string_view>

namespace noam {
template <class T>
class co_parse;

// workaround until I figure out how to make co_parse idempotent
template <class F>
F& move_if_necessary(F& f) {
    return f;
}

template <class T>
co_parse<T>&& move_if_necessary(co_parse<T>& p) {
    return std::move(p);
}

template <class Func>
struct await_parse {
    Func func {};
    // This is provided by the promise type via await_transform
    // It's non-owned so using a pointer here is fine
    state_t* state = nullptr;

    using result_t = decltype(std::declval<Func>().parse(state_t {}));
    result_t result;
    // Gets a copy of the internal state
    constexpr state_t copy_state() const noexcept { return *state; }
    constexpr bool await_ready() noexcept {
        result = std::forward<Func>(func).parse(copy_state());
        if (result.good()) {
            // We only update the state if the parse succeeded
            *state = result.get_state();
            return true;
        } else {
            // Don't update the state
            return false;
        }
    }
    // If the result wasn't good, we're shutting the whole thing down.
    // await_suspend is suspending the whole-ass m'fucking operation
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    constexpr decltype(auto) await_resume() & noexcept {
        return std::forward<decltype(result)>(result).get_value();
    }
    constexpr decltype(auto) await_resume() const& noexcept {
        return result.get_value();
    }
    constexpr decltype(auto) await_resume() && noexcept {
        return std::move(*this).result.get_value();
    }
};
template <class F>
await_parse(F func, state_t*) -> await_parse<F>;

template <any_parser... F>
auto either(F&&... funcs) {
    using result_t = std::common_type_t<std::invoke_result_t<F, state_t>...>;
    return [... f = std::forward<F>(funcs)](state_t state) -> result_t {
        result_t result;
        // get results until one of them returns true
        ((result = move_if_necessary(f)(state_t(state))).good() || ...);
        return result;
    };
}

template <class Value>
struct co_parse_result {
    state_t state_ {};
    Value value_ {};
    bool has_result = false;
    constexpr bool good() const noexcept { return has_result; }
    constexpr state_t get_state() const noexcept { return state_; }
    constexpr decltype(auto) get_value() & noexcept { return value_; }
    constexpr decltype(auto) get_value() const& noexcept { return value_; }
    constexpr decltype(auto) get_value() && noexcept {
        return std::move(*this).value_;
    }
};

template <class T>
struct parser_promise {
    co_parse_result<T> result;
    state_t current_state;
    void set_initial_state(state_t state) {
        result.state_ = state;
        current_state = state;
    }

    constexpr std::suspend_always initial_suspend() noexcept { return {}; }
    constexpr std::suspend_always final_suspend() noexcept { return {}; }

    void return_value(T value) {
        result.value_ = std::move(value);
        result.has_result = true;
        result.state_ = current_state;
    }
    template <any_parser F>
    auto await_transform(F&& func) {
        return await_parse<std::decay_t<F>> {
            std::forward<F>(func), &current_state};
    };

    template <class U>
    auto await_transform(co_parse<U> (*_coro_ptr)()) {
        return await_parse {_coro_ptr(), &current_state};
    }

    void unhandled_exception() {
        // Fuck this shit we just cancelling it all
        result.has_result = false;
    }

    co_parse<T> get_return_object() {
        return co_parse<T>(
            std::coroutine_handle<parser_promise>::from_promise(*this));
    }
};
template <class T>
class co_parse {
    std::coroutine_handle<parser_promise<T>> handle_;

   public:
    using promise_type = parser_promise<T>;
    co_parse() = default;
    co_parse(co_parse&& d) noexcept
      : handle_(std::exchange(d.handle_, nullptr)) {}
    co_parse(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

    // This should only ever be run once, so it must be run either
    // on a prvalue co_parse or a moved co_parse
    co_parse_result<T> operator()(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise().result);
    }
    co_parse_result<T> parse(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise().result);
    }
    ~co_parse() { handle_.destroy(); }
};
} // namespace noam
