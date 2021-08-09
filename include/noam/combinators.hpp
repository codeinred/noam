#pragma once
#include <noam/concepts.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>

// This file holds functios that transform or modify parsers

namespace noam {
/**
 * @brief Function that takes a value and returns a pure parser that returns
 * that value
 *
 * @param value the value to return when invoking the parser
 */
constexpr auto pure = [](auto value) {
    return noam::parser {[=](std::string_view state) {
        return noam::pure_result {state, value};
    }};
};

/**
 * @brief Creates a parser p and returns a parser that reads values parsed by p
 * and combines them using fold
 *
 * @param p the parser to fold
 * @param fold the operation with which to combine values produced by p
 * @return parser<(fold_left1:lambda)> a parser that repeatedly obtains values
 * by parsing the input with p, then folds those values using fold
 */
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
