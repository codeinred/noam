#pragma once
#include <noam/concepts.hpp>
#include <noam/coupling/parse_promise.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <noam/parser.hpp>
#include <optional>
#include <string_view>

namespace noam {
template <class T>
class parser<parse_promise<T>> {
    using handle_t = std::coroutine_handle<parse_promise<T>>;
    handle_t handle_;

   public:
    using promise_type = parse_promise<T>;
    parser() = default;
    parser(parser&& d) noexcept
      : handle_(std::exchange(d.handle_, nullptr)) {}
    parser(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

    // This should only ever be run once, so it must be run either
    // on a prvalue parser or a moved parser
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
    ~parser() { handle_.destroy(); }
};

template <class T>
using co_parse = parser<parse_promise<T>>;
} // namespace noam
