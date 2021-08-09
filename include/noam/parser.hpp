#pragma once
#include <noam/concepts.hpp>

namespace noam {
// Type which represents a parser
// You can convert any lambda l: state_t -> parse_result into a parser
// Simply by invoking the constructor via parser{l}
template <class Func>
struct parser {
    // Enables empty base class optimization
    // if Func is empty (i.e, [](){}), parser<Func> is too
    [[no_unique_address]] Func parse;
};
template <class Func>
parser(Func) -> parser<Func>;

constexpr auto make_parser = []<class T>(T&& implementation) {
    return parser {std::forward<T>(implementation)};
};
} // namespace noam
