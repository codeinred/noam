#pragma once
#include <noam/type_traits.hpp>
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

namespace noam {
/**
 * @brief Takes a parsers p1...pn and produces a parser that returns a value of
 * T constructed from the values parsed by p1...pn.
 *
 * @tparam T the type the parser should construct
 * @tparam Parsers the types of the parsers used to obtain the arguments to T's
 * constructor
 * @param parsers the parsers used to obtain the arguments to T's constructor
 * @return parser returns a parser
 */
template <class T, class... Parsers>
constexpr auto make_w_coro(Parsers&&... parsers) {
    return parser {
        [... parsers =
             std::forward<Parsers>(parsers)](state_t st) -> noam::result<T> {
            co_return T {(co_await parsers)...};
        }};
}
} // namespace noam
