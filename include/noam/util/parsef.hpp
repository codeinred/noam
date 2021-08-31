#pragma once

#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/type_traits.hpp>
#include <noam/util/helpers.hpp>
#include <tuplet/tuple.hpp>

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
                if (auto post = postfix.parse(value.get_state())) {
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

template <class... Parsers>
struct match : tuplet::tuple<Parsers...> {
    using base = tuplet::tuple<Parsers...>;
    using base_list = typename base::base_list;
    constexpr static bool always_good = (parser_always_good_v<Parsers> && ...);
    using result_type =
        std::conditional_t<always_good, pure_result<empty>, result<empty>>;
    template <class... Bases>
    constexpr auto parse_impl(state_t st, tuplet::type_list<Bases...>) const
        -> result_type {
        if constexpr (always_good) {
            (update_state(Bases::value.parse(st), st), ...);
            return {st, empty {}};
        } else {
            if ((update_state(Bases::value.parse(st), st) && ...)) {
                return {st, empty {}};
            } else {
                return {};
            }
        }
    }
    constexpr auto operator()(state_t st) const -> result_type {
        return parse_impl(st, base_list{});
    }
};

template <class... P>
struct join : meta::all_but_last_t<match, P...> {
    using match_t = meta::all_but_last_t<match, P...>;
    using last_parser = meta::last_t<P...>;
    using result_type = std::conditional_t<
        match_t::always_good,
        parser_result_t<last_parser>,
        result<parser_value_t<last_parser>>>;
    last_parser p;
    template <class... Bases>
    constexpr auto operator()(state_t st) const -> result_type {
        if constexpr (match_t::always_good) {
            return p.parse(match_t::parse_impl(st, typename match_t::base_list{}).get_state());
        } else {
            if (auto r = match_t::parse_impl(st, typename match_t::base_list{})) {
                return p.parse(r.get_state());
            } else {
                return {};
            }
        }
    }
};
template <class... T>
join(T...) -> join<T...>;
} // namespace noam::parsef
