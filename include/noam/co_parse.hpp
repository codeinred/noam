#pragma once
#include <noam/concepts.hpp>
#include <noam/coupling/parse_promise.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <optional>
#include <string_view>

namespace noam {
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
