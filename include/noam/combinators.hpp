#pragma once
#include <noam/concepts.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>

// This file holds functios that transform or modify parsers

namespace noam {
auto fold_left = [](auto p, auto fold) {
    using value_t = std::decay_t<decltype(p.parse(state_t {}).value())>;
    return parser {[=](state_t state) -> noam::standard_result<value_t> {
        auto result = p.parse(state);
        if (!result.good()) {
            return {};
        }
        state = result.new_state();
        value_t value = result.value();
        for (result = p.parse(state); result.good(); result = p.parse(state)) {
            value = fold(value, result.value());
            state = result.new_state();
        }
        return noam::standard_result {state, value};
    }};
};

template <class F, any_parser Parser>
auto map(F&& func, Parser&& p) {
    return parser {[f = std::forward<F>(func),
                    p = std::forward<Parser>(p)](state_t state) {
        // to-do: implement map on parser_result
        return transform_result {p.parse(state), f};
    }};
}

constexpr auto test = [](auto&& any_parser) {
    return parser {[=](state_t sv) -> boolean_result {
        return boolean_result(sv, any_parser(sv));
    }};
};

} // namespace noam
