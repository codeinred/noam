#pragma once

#include <noam/concepts.hpp>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>

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

/**
 * @brief Implements a functor that will compare a series of parsers
 *
 * @tparam Parsers The parsers to apply either over
 */
template <class... Parsers>
struct either;
template <>
struct either<> {
    /**
     * @brief An empty either always returns a bad result
     *
     * @return result<empty> an empty result type representing a bad result
     */
    constexpr auto operator()(state_t) const noexcept {
        return result<empty> {};
    }
};
template <class Parser>
struct either<Parser> {
    Parser parser;
    using value_type = parser_value_t<Parser>;
    using result_type = parser_result_t<Parser>;
    constexpr auto operator()(state_t st) const
        noexcept(noexcept(parser.parse(st))) {
        return parser.parse(st);
    }
};
template <class PA, class PB>
struct either<PA, PB> {
    using resultA = parser_result_t<PA>;
    using resultB = parser_result_t<PB>;
    PA parserA;
    PB parserB;
    constexpr static bool always_good =
        result_always_good_v<resultA> || result_always_good_v<resultB>;
    using value_type =
        std::common_type_t<result_value_t<resultA>, result_value_t<resultB>>;
    using result_type = std::
        conditional_t<always_good, pure_result<value_type>, result<value_type>>;
    constexpr auto operator()(state_t st) const noexcept(
        noexcept(value_type(parserA.parse(st).get_value())) && noexcept(
            value_type(parserB.parse(st).get_value()))) -> result_type {
        if (auto res = parserA.parse(st)) {
            return {res.get_state(), std::move(res).get_value()};
        }
        if constexpr (std::constructible_from<result_type, resultB&&>) {
            return result_type(parserB.parse(st));
        } else {
            if (auto res = parserB.parse(st)) {
                return {res.get_state(), std::move(res).get_value()};
            } else {
                return {};
            }
        }
    }
};
template <class PA, class... PB>
struct either<PA, PB...> {
    PA first;
    either<PB...> rest;
    using resultA = noam::parser_result_t<PA>;
    using resultB = typename either<PB...>::result_type;
    constexpr static bool always_good =
        result_always_good_v<resultA> || result_always_good_v<resultB>;
    using value_type =
        std::common_type_t<result_value_t<resultA>, result_value_t<resultB>>;
    using result_type = std::
        conditional_t<always_good, pure_result<value_type>, result<value_type>>;
    constexpr auto operator()(state_t st) const -> result_type {
        if (auto res = first.parse(st)) {
            if constexpr (std::constructible_from<result_type, resultA&&>) {
                return result_type(std::move(res));
            } else {
                return {res.get_state(), std::move(res).get_value()};
            }
        }
        if constexpr (std::constructible_from<result_type, resultB&&>) {
            return result_type(rest(st));
        } else {
            if (auto res = rest(st)) {
                return {res.get_state(), std::move(res).get_value()};
            } else {
                return {};
            }
        }
    }
};
template <class... Parser>
either(Parser...) -> either<Parser...>;

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
