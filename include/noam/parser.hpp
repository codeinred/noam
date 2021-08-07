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
class do_parse;

template <class T>
concept parse_result = requires(T t) {
    { t.good() } -> convertible_to<bool>;
    { t.value() };
    { t.new_state() } -> same_as<state_t>;
};

class boolean_result {
    std::string_view state;
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

constexpr auto test = [](auto&& parser_func) {
    return [=](std::string_view sv) -> boolean_result {
        return boolean_result(sv, parser_func(sv));
    };
};

template <class F>
concept parser_func = requires(F func, state_t state) {
    { func(state) } -> parse_result;
};

// workaround until I figure out how to make do_parse idempotent
template <class F>
F& move_if_necessary(F& f) {
    return f;
}

template <class T>
do_parse<T>&& move_if_necessary(do_parse<T>& p) {
    return std::move(p);
}

template <class Func>
struct await_parse {
    Func func {};
    // This is provided by the promise type via await_transform
    // It's non-owned so using a pointer here is fine
    state_t* state = nullptr;

    using result_t = std::invoke_result_t<Func, state_t>;
    result_t result;
    // Gets a copy of the internal state
    constexpr state_t copy_state() const noexcept { return *state; }
    constexpr bool await_ready() noexcept {
        result = std::forward<Func>(func)(copy_state());
        if (result.good()) {
            // We only update the state if the parse succeeded
            *state = result.new_state();
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
        return std::forward<decltype(result)>(result).value();
    }
    constexpr decltype(auto) await_resume() const& noexcept {
        return result.value();
    }
    constexpr decltype(auto) await_resume() && noexcept {
        return std::move(*this).result.value();
    }
};
template <class F>
await_parse(F func, state_t*) -> await_parse<F>;

template <parser_func... F>
auto either(F&&... funcs) {
    using result_t = std::common_type_t<std::invoke_result_t<F, state_t>...>;
    return [... f = std::forward<F>(funcs)](state_t state) -> result_t {
        result_t result;
        // get results until one of them returns true
        ((result = move_if_necessary(f)(state_t(state))).good() || ...);
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
    constexpr state_t new_state() const noexcept { return state_; }
    constexpr decltype(auto) value() & noexcept { return value_; }
    constexpr decltype(auto) value() const& noexcept { return value_; }
    constexpr decltype(auto) value() && noexcept {
        return std::move(*this).value_;
    }
};

template <class T>
struct parser_promise {
    do_parse_result<T> result;
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
    template <parser_func F>
    auto await_transform(F&& func) {
        return await_parse<std::decay_t<F>> {std::forward<F>(func), &current_state};
    };

    template <class U>
    auto await_transform(do_parse<U> (*_coro_ptr)()) {
        return await_parse {_coro_ptr(), &current_state};
    }

    void unhandled_exception() {
        // Fuck this shit we just cancelling it all
        result.has_result = false;
    }

    do_parse<T> get_return_object() {
        return do_parse<T>(
            std::coroutine_handle<parser_promise>::from_promise(*this));
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
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise().result);
    }
    do_parse_result<T> parse(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise().result);
    }
    ~do_parse() { handle_.destroy(); }
};
} // namespace noam
