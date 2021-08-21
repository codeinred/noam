#pragma once
#include <noam/concepts.hpp>
#include <noam/coupling/parse_promise.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <optional>
#include <string_view>

template <class T, class Functor>
struct std::coroutine_traits<noam::result<T>, Functor, noam::state_t> {
    using promise_type = noam::parse_promise<T>;
};
template <class T>
struct std::coroutine_traits<noam::result<T>, noam::state_t> {
    using promise_type = noam::parse_promise<T>;
};
