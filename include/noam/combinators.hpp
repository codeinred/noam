#pragma once
#include <noam/concepts.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/then_operator.hpp>

// This file holds functios that return parsers based on inputs

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

/**
 * @brief Maps a function over a parser, returning a new parser whose output is
 * func applied to the output of p
 *
 * @tparam F
 * @tparam Parser
 * @param func The function used to do the map operation
 * @param p The parser to do the map operation on
 * @return parser<(map:lambda)> returns a parser such that map(func,
 * p).parse(str).value() = func(p.parse(str)).value())
 */
template <class F, any_parser Parser>
auto map(F&& func, Parser&& p) {
    return parser {[f = std::forward<F>(func),
                    p = std::forward<Parser>(p)](state_t state) {
        return transform_result {p.parse(state), f};
    }};
}

/**
 * @brief Takes a parser p and produces a new parser that generates a true if p
 * succeeded and false if p failed
 *
 * @param p A parser whose output you wish to test
 * @return parser<(lambda)> A new parser
 */
constexpr auto test = [](auto&& p) {
    return parser {[=](state_t sv) -> boolean_result {
        return boolean_result(sv, p(sv));
    }};
};

/**
 * @brief Creates a parser that returns true if a prefix is matched and false
 * otherwise
 *
 * @param prefix the prefix to test
 */
constexpr auto test_prefix = [](std::string_view prefix) {
    return [=](noam::state_t state) {
        if (state.starts_with(prefix)) {
            return boolean_result {state.substr(prefix.size()), true};
        } else {
            return boolean_result {state, false};
        }
    } / make_parser;
};

/**
 * @brief Creates a parser that requires a prefix. It'll return the prefix
 * parsed if matched, and fail otherwise
 *
 * @param prefix input to match
 *
 */
constexpr auto require_prefix = [](std::string_view prefix) {
    return [=](noam::state_t state) -> standard_result<std::string_view> {
        if (state.starts_with(prefix)) {
            return {state.substr(prefix.size()), prefix};
        } else {
            return {};
        }
    } / make_parser;
};

/**
 * @brief Parses a value with lookahead, so that no part of the string is
 * actually consumed
 *
 */
constexpr auto lookahead = [](auto&& parser) {
    using value_t = std::decay_t<decltype(parser.parse(state_t {}).value())>;
    return [parser = std::forward<decltype(parser)>(parser)](state_t state) {
        auto result = parser.parse(state);
        if constexpr (result_always_good_v<decltype(result)>) {
            return pure_result {state, std::move(result).value()};
        } else {
            if (result.good()) {
                return standard_result<value_t> {state, std::move(result).value()};
            } else {
                return standard_result<value_t> {};
            }
        }
    } / make_parser;
};
} // namespace noam
