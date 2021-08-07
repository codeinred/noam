#include <concepts>
#include <coroutine>
#include <optional>
#include <string_view>

namespace noam {
using std::convertible_to;
using std::invocable;
using std::same_as;
using state_t = std::string_view;

template <class T>
concept parse_result =
    std::is_default_constructible_v<T> && requires(T const t) {
    { t.good() } -> convertible_to<bool>;
    { t.value() };
    { t.state() } -> same_as<state_t>;
};
template <class F>
concept parser_func = requires(F func, state_t state) {
    { func(state) } -> parse_result;
};

template <parser_func Func>
struct await_parse {
    Func func {};
    // This is provided by the promise type via await_transform
    // It's non-owned so using a pointer here is fine
    state_t* state = nullptr;

    // Gets a copy of the internal state
    constexpr state_t copy_state() const noexcept { return *state; }
    using result_t = std::invoke_result_t<Func, state_t>;
    result_t result;
    constexpr bool await_ready() const noexcept {
        result = std::forward<Func>(func)(copy_state());
        if (result.good()) {
            // We only update the state if the parse succeeded
            *state = result.state();
            return true;
        } else {
            // Don't update the state
            return false;
        }
    }
    // If the result wasn't good, we're shutting the whole thing down.
    // await_suspend is suspending the whole-ass m'fucking operation
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    constexpr decltype(auto) await_resume() const noexcept {
        return std::forward<result_t>(result).value();
    }
};
template <class F>
await_parse(F func, state_t*) -> await_parse<F>;
template <class F>
await_parse(F func, state_t*, std::invoke_result_t<F, state_t>)
    -> await_parse<F>;

template <parser_func... F>
auto either(F&&... funcs) {
    using result_t = std::common_type_t<std::invoke_result_t<F, state_t>...>;
    return [... f = std::forward<F>(funcs)](state_t state) -> result_t {
        result_t result;
        // get results until one of them returns true
        ((result = f(state_t(state))).good() || ...);
        return result;
    };
}

template <class F, parser_func Parser>
auto map(F&& func, Parser&& parser) {
    return [f = std::forward<F>(func),
            p = std::forward<Parser>(parser)](state_t state) {
        // to-do: implement map on parser_result
    };
}

template <class Value>
struct do_parse_result {
    state_t state_ {};
    Value value_ {};
    bool has_result = false;
    constexpr bool good() const noexcept { return has_result; }
    constexpr state_t state() const noexcept { return state_; }
    constexpr decltype(auto) value() & noexcept { return value_; }
    constexpr decltype(auto) value() const& noexcept { return value_; }
    constexpr decltype(auto) value() && noexcept {
        return std::move(*this).value_;
    }
};

template <class T>
struct parser_promise {
    do_parse_result<T> result;

    constexpr std::suspend_always initial_suspend() noexcept { return {}; }
    constexpr std::suspend_always final_suspend() noexcept { return {}; }

    void return_value(T value) {
        result.value_ = std::move(value);
        result.has_result = true;
    }
    template <parser_func F>
    auto await_transform(F&& func) {
        return await_parse {std::forward<F>(func), &result.state_, {}};
    };
    void unhandled_exception() {
        // Fuck this shit we just cancelling it all
        result.has_result = false;
    }
};
template <class T>
class do_parse {
    std::coroutine_handle<parser_promise<T>> handle_;

   public:
    using promise_type = parser_promise<T>;
    do_parse() = default;
    do_parse(do_parse&& d) noexcept
      : handle_(std::exchange(d.handle_, nullptr)) {}
    do_parse(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

    // This should only ever be run once, so it must be run either
    // on a prvalue do_parse or a moved do_parse
    do_parse_result<T> operator()(state_t state) && {
        handle_.resume();
        return std::move(handle_.promise().result);
    }
    ~do_parse() {
        handle_.destroy();
    }
};
} // namespace noam
