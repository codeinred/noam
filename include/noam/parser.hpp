#pragma once
#include <noam/type_traits.hpp>
#include <optional>
#include <type_traits>
#include <utility>

namespace noam {
// Type which represents a parser
// You can convert any lambda l: state_t -> parse_result into a parser
// Simply by invoking the constructor via parser{l}
template <class Func>
struct parser {
    // Enables empty base class optimization
    // if Func is empty (i.e, [](){}), parser<Func> is too
    [[no_unique_address]] Func parse;

    auto read(noam::state_t& st) const {
        if (auto res = parse(st)) {
            st = res.get_state();
            return res;
        } else {
            return decltype(res){};
        }
    }
};

template <any_parser Base>
struct parser<Base> : Base {
    using Base::parse;

    auto read(noam::state_t& st) const {
        if (auto res = parse(st)) {
            st = res.get_state();
            return res;
        } else {
            return decltype(res){};
        }
    }
};
template <class Func>
parser(Func) -> parser<Func>;
template <class Func>
parser(parser<Func>) -> parser<Func>;

/**
 * @brief This takes an implementation (such as a function or coroutine) and
 * returns a parser based on the given implementation
 *
 * @param implementation The parameter or value defining the implementation
 */
constexpr auto make_parser = []<class T>(T&& implementation) {
    return parser {std::forward<T>(implementation)};
};
} // namespace noam
