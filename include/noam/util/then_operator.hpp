#pragma once
#include <concepts>
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
} // namespace noam
