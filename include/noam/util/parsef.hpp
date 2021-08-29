#pragma once

#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/type_traits.hpp>
#include <noam/util/helpers.hpp>

// This file holds functios that return parsers based on inputs

/**
 * @brief noam::parsef contains the definitions of parser functors. These
 * represent the implementation of various combinators
 *
 */
namespace noam::parsef {
template <class Value>
struct pure {
    [[no_unique_address]] Value value {};
    constexpr auto operator()(state_t state) const
        noexcept(noexcept(noam::pure_result<Value> {state, value})) {
        return noam::pure_result<Value> {state, value};
    }
};
template <class Value>
pure(Value) -> pure<Value>;

template <class Func, class Parser>
struct map {
    [[no_unique_address]] Func func;
    [[no_unique_address]] Parser parser;
    using result_before_map = parser_result_t<Parser>;
    constexpr static bool always_good = result_always_good_v<result_before_map>;
    using value_type = std::invoke_result_t<Func, parser_value_t<Parser>>;
    constexpr auto operator()(state_t st) const {
        if constexpr (always_good) {
            auto result = parser.parse(st);
            return pure_result<value_type> {
                result.get_state(), func(std::move(result).get_value())};
        } else {
            if (auto result = parser.parse(st)) {
                return noam::result<value_type> {
                    result.get_state(), func(std::move(result).get_value())};
            } else {
                return noam::result<value_type> {};
            }
        }
    }
};

template <class Prefix, class Value, class Postfix>
struct surround {
    [[no_unique_address]] Prefix prefix {};
    [[no_unique_address]] Value parser {};
    [[no_unique_address]] Postfix postfix {};
    constexpr auto operator()(state_t st) const {
        using value_type = parser_value_t<Value>;
        if (auto pre = prefix.parse(st)) {
            if (auto value = parser.parse(pre.get_state())) {
                if (auto post = prefix.parse(value.get_state())) {
                    return noam::result<value_type> {
                        post.get_state(), std::move(value).get_value()};
                }
            }
        }
        return noam::result<value_type> {};
    }
};
template <class A, class B, class C>
surround(A, B, C) -> surround<A, B, C>;
} // namespace noam::parsef
