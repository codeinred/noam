#pragma once
#include <noam/type_traits.hpp>

namespace noam {
/**
 * @brief Attempts to apply a parser, placing the result in r. On success,
 * returns true and updates the state. On failure, returns false, and the state
 * is not updated.
 *
 * @tparam P the type of the parser being passed to parse_asign
 * @tparam Result the type of the result that will be assigned the output of
 * parser.parse(st)
 * @param st the state
 * @param parser the parser to apply to the state
 * @param r the result which will be assigned the output of parser.parse(st)
 * @return true if the parser succeeded
 * @return false if the parser failed
 */
template <class P, class Result>
constexpr bool parse_assign(state_t& st, P const& parser, Result& r) {
    if constexpr (parser_always_good_v<P>(parser)) {
        r = parser.parse(st);
        st = r.get_state();
        return true;
    } else {
        if ((r = parser.parse(st))) {
            st = r.get_state();
            return true;
        } else {
            return false;
        }
    }
}
} // namespace noam
