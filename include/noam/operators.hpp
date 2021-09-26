#pragma once
#include <concepts>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <utility>

namespace noam {
/**
 * @brief Takes an input and a func and applies the func to the input
 *
 * @tparam Input Type of input
 * @tparam Func Type of func (must be invocable on input)
 * @param input value of input
 * @param func value of func
 * @return constexpr auto returns a type identical to func(input). func and
 * input are both forwarded.
 */
template <class Input, std::invocable<Input> Func>
constexpr auto operator/(Input&& input, Func&& func) {
    return std::forward<Func>(func)(std::forward<Input>(input));
}

template <class Parser1, class Parser2>
constexpr auto operator>>(Parser1&& p1, Parser2&& p2) {
    return [p1 = std::forward<Parser1>(p1), p2 = std::forward<Parser2>(p2)](
               state_t state) -> result<parser_value_t<Parser2>> {
        auto r1 = p1.parse(state);
        if (r1) {
            auto r2 = p2.parse(r1.get_state());
            if (r2) {
                return {r2.get_state(), std::move(r2).get_value()};
            } else {
                return {};
            }
        } else {
            return {};
        }
    } / make_parser;
}
} // namespace noam
