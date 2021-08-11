#pragma once
#include <noam/coupling/await_parser.hpp>
#include <noam/result_types.hpp>
#include <noam/parser.hpp>
#include <tuple>


namespace noam {
template <class T>
struct parse_promise {
    using handle_t = std::coroutine_handle<parse_promise>;
    state_t current_state;
    T value;
    bool is_good = false;
    /**
     * @brief This function pointer provides a means to get a new coroutine
     * frame representing a fresh parser
     *
     */
    handle_t (*create_new_parser_frame)(parse_promise&) = nullptr;
    void set_initial_state(state_t state) { current_state = state; }

    parse_promise(handle_t (*create_new_parser_frame)(parse_promise&))
      : create_new_parser_frame(create_new_parser_frame) {}

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
        return await_parser<F> {
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

    handle_t clone_frame() {
        return create_new_parser_frame(*this);
    }
};

template <class T, class Coro, class... Args>
struct parse_promise_specialization : parse_promise<T> {
    using base = parse_promise<T>;
    using self_t = parse_promise_specialization;

    using base::await_transform;
    using base::create_new_parser_frame;
    using base::final_suspend;
    using base::get_parse_result;
    using base::get_return_object;
    using base::initial_suspend;
    using base::return_value;
    using base::set_initial_state;
    using base::unhandled_exception;
    using typename base::handle_t;

    handle_t clone_frame_impl() {
        return std::decay_t<Coro>{}().release_handle();
    }

    static handle_t create_new_parser_frame_impl(base& this_base) {
        return static_cast<parse_promise_specialization&>(this_base)
            .clone_frame_impl();
    }

    parse_promise_specialization() : base(create_new_parser_frame_impl) {}
};
} // namespace noam
