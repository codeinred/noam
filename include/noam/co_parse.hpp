#pragma once
#include <noam/concepts.hpp>
#include <noam/coupling/parse_promise.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <optional>
#include <string_view>

namespace noam {
template <class T>
class parser<parse_promise<T>> {
    using handle_t = std::coroutine_handle<parse_promise<T>>;
    handle_t handle_ = nullptr;

   public:
    using promise_type = parse_promise<T>;
    parser() = default;
    parser(parser&& d) noexcept
      : handle_(std::exchange(d.handle_, nullptr)) {}
    parser(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

    handle_t release_handle() && {
      return std::exchange(handle_, nullptr);
    }

    standard_result<T> parse(state_t state) const& {
        return parser(handle_.promise().clone_frame()).parse(state);
    }
    /**
     * @brief Parses the input state, returning a standard_result<T>
     *
     * This should only ever be run once, so it must be run either on a prvalue
     * parser or a moved parser. TODO: Fix this by specializing parse_promise so
     * that it can produce a new instance of the coroutine frame that's been
     * reset back to the initial state of the parser
     *
     * @param state the string or substring being parsed
     * @return standard_result<T>
     */
    standard_result<T> parse(state_t state) && {
        handle_.promise().set_initial_state(state);
        handle_.resume();
        return std::move(handle_.promise()).get_parse_result();
    }
    ~parser() { if(handle_) handle_.destroy(); }
};

template <class T>
using co_parse = parser<parse_promise<T>>;
} // namespace noam

template <class T, class... Args>
struct std::coroutine_traits<noam::co_parse<T>, Args...> {
    using promise_type = noam::parse_promise_specialization<T, Args...>;
};
