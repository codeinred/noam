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
struct co_result : standard_result<T> {
    using value_type = T;
    using base_t = standard_result<T>;
    using promise_t = parse_promise<T>;
    using handle_t = std::coroutine_handle<promise_t>;
    using base_t::operator bool;
    using base_t::get_state;
    using base_t::get_value;
   private:
    static base_t run_handle(handle_t handle) {
        handle.resume();
        promise_t& promise = handle.promise();
        if (promise) {
            state_t state = promise.get_state();
            return base_t{state, std::move(promise).get_value()};
        } else {
            return base_t{};
        }
    }

   public:
    co_result() = default;
    co_result(handle_t handle)
      : base_t(run_handle(handle)) {
        handle.destroy();
    }
    co_result(co_result const&) = default;
    co_result(co_result&&) = default;
    co_result& operator=(co_result const&) = default;
    co_result& operator=(co_result&&) = default;
};
} // namespace noam

template <class T, class Functor>
struct std::coroutine_traits<noam::co_result<T>, Functor, noam::state_t> {
    using promise_type = noam::parse_promise<T>;
};
template <class T>
struct std::coroutine_traits<noam::co_result<T>, noam::state_t> {
    using promise_type = noam::parse_promise<T>;
};
