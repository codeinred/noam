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
namespace noam {
template <class Value>
using pure_parser = parser<parsef::pure<Value>>;
/**
 * @brief Function that takes a value and returns a pure parser that returns
 * that value
 *
 * @param value the value to return when invoking the parser
 */
template <class Value>
constexpr auto pure(Value&& value) {
    return parser {parsef::pure {std::forward<Value>(value)}};
}

/**
 * @brief Creates a parser parser and returns a parser that reads values parsed
 * by parser and combines them using fold
 *
 * @param parser the parser to fold
 * @param op the operation with which to combine values produced by parser
 * @return parser<(fold_left:lambda)> a parser that repeatedly obtains values
 * by parsing the input with parser, then folds those values using fold
 */
template <class Parser1, class Parser2, class Op>
constexpr auto fold_left(Parser1&& initial, Parser2&& rest, Op&& op) {
    using value_t = parser_value_t<Parser1>;
    return [initial = std::forward<Parser1>(initial),
            rest = std::forward<Parser2>(rest),
            op = std::forward<Op>(op)](state_t state) -> noam::result<value_t> {
        if (auto initial_result = initial.parse(state)) {
            value_t value = initial_result.get_value();
            state = initial_result.get_state();
            while (auto next_result = rest.parse(state)) {
                value = op(value, next_result.get_value());
                state = next_result.get_state();
            }
            return noam::result {state, value};
        }
        return {};
    } / make_parser;
}

/**
 * @brief Maps a function over a parser, returning a new parser whose output is
 * func applied to the output of parser
 *
 * @tparam Func
 * @tparam Parser
 * @param func The function used to do the map operation
 * @param parser The parser to do the map operation on
 * @return parser<(map:lambda)> returns a parser such that map(func,
 * parser).parse(str).get_value() = func(parser.parse(str)).get_value())
 */
template <class Func, any_parser Parser>
constexpr auto map(Func&& func, Parser&& parser) {
    return parsef::map {std::forward<Func>(func), std::forward<Parser>(parser)};
}

/**
 * @brief Creates a backtracking parser that will test each parser in sequence
 *
 * @tparam Parsers the types of the parsers to test
 * @param parsers the parsers to test
 * @return parser A parser that will test each input in sequence, returning the
 * first successful result
 */
template <class... Parsers>
constexpr auto either(Parsers&&... parsers) {
    return parser {parsef::either {std::forward<Parsers>(parsers)...}};
}

/**
 * @brief Parses a value surrounded by a prefix given by Parser `pre` and a
 * postfix given by Parser `post`.
 *
 * @tparam Prefix type of the parser for the prefix
 * @tparam Parser type of the parser for the value
 * @tparam Postfix type of the parser for the postfix
 * @param pre parser for the prefix
 * @param par parser for the value
 * @param post parser for the postfix
 * @return parser
 */
template <class Prefix, class Parser, class Postfix>
constexpr auto surround(Prefix&& pre, Parser&& par, Postfix&& post) {
    return parser {parsef::surround {
        std::forward<Prefix>(pre),
        std::forward<Parser>(par),
        std::forward<Postfix>(post)}};
}

template <class... First, class Last>
constexpr auto join(First&&... first, Last&& last) {
    constexpr bool first_always_good = (parser_always_good_v<First> && ...);
    return [... first = std::forward<First>(first),
            last = std::forward<Last>(last)](state_t st) {
        if constexpr (first_always_good) {
            ((st = first.parse(st).get_state()), ...);
            return last.parse(st);
        } else {
            auto try_to_parse = [&](auto& parser) -> bool {
                if (auto result = parser.parse(st)) {
                    st = result.get_state();
                    return true;
                }
                return false;
            };
            using value_type = parser_value_t<Last>;
            using result_type = result<value_type>;
            using last_result_type = parser_result_t<Last>;
            if ((try_to_parse(first) && ...)) {
                if constexpr (std::constructible_from<
                                  result_type,
                                  last_result_type>) {
                    return result_type(last.parse(st));
                } else {
                    if (auto res = last.parse(st)) {
                        return result_type {res.get_state(), res.get_value()};
                    } else {
                        return result_type {};
                    }
                }
            }
        }
    };
}
/**
 * @brief Takes a parser parser and produces a new parser that generates a true
 * if parser succeeded and false if parser failed
 *
 * @param parser A parser whose output you wish to test
 * @return parser<(lambda)> A new parser
 */
template <class Parser>
constexpr auto test(Parser&& parser) {
    return
        [parser = std::forward<Parser>(parser)](state_t sv) -> boolean_result {
            // Note that boolean_result will select parser(sv).get_state() if
            // parser(sv) is good, and as a result no check needs to be done
            // here
            return boolean_result(sv, parser.parse(sv));
        } / make_parser;
}

/**
 * @brief Takes `parser` and produces a new parser that generates true if
 * the parser succeeds, and false if the parser fails. If the parser
 * succeeded, func is invoked with parser.parse(...).get_value().
 *
 *
 * @param parser The parser to test
 * @param func the func to call on the value produced by the parser, when
 * good
 * @return parser A
 */
template <class Parser, class Func>
constexpr auto test_then(Parser&& parser, Func&& func) {
    return [parser = std::forward<Parser>(parser),
            func = std::forward<Func>(func)](state_t state) -> boolean_result {
        auto result = parser.parse(state);
        if (result) {
            state = result.get_state();
            func(std::move(result).get_value());
            return boolean_result {state, true};
        } else {
            return boolean_result {state, false};
        }
    } / make_parser;
}

/**
 * @brief Takes a parser parser and produces a new parser that generates a
 * true if parser succeeded and false if parser failed. Does so with
 * lookahead, so that no portion of the string is consumed
 *
 * @param parser A parser whose output you wish to test
 * @return parser<(lambda)> A new parser
 */
template <class Parser>
constexpr auto test_lookahead(Parser&& parser) {
    return
        [parser = std::forward<Parser>(parser)](state_t sv) -> boolean_result {
            return boolean_result(sv, parser.parse(sv));
        } / make_parser;
}

/**
 * @brief Creates a parser that returns true if a prefix is matched and
 * false otherwise
 *
 * @param prefix the prefix to test
 */
constexpr auto test_prefix(state_t prefix) {
    return [=](state_t state) {
        if (state.starts_with(prefix)) {
            return boolean_result {state.substr(prefix.size()), true};
        } else {
            return boolean_result {state, false};
        }
    } / make_parser;
}

/**
 * @brief Creates a parser that requires a prefix. It'll return the prefix
 * parsed if matched, and fail otherwise
 *
 * @param prefix input to match
 *
 */
constexpr auto require_prefix(state_t prefix) {
    return [=](state_t state) -> result<state_t> {
        if (state.starts_with(prefix)) {
            return {state.substr(prefix.size()), prefix};
        } else {
            return {};
        }
    } / make_parser;
}

/**
 * @brief Parses a value with lookahead, so that no part of the string is
 * actually consumed
 *
 * @param parser the parser being given to the combinator to be transformed
 */
template <class Parser>
constexpr auto lookahead(Parser&& parser) {
    using result_t = parser_result_t<Parser>;
    using value_t = parser_value_t<Parser>;
    return [parser = std::forward<Parser>(parser)](state_t state) {
        result_t result = parser.parse(state);

        // If the result is lookahead_enabled, we can simply reset the state
        // of the result without transforming it's type. Otherwise, we'll
        // transform it into either a pure_result (if it's always good), or
        // a result (if it may not always be good)
        if constexpr (lookahead_enabled_result<result_t>) {
            result.set_state(state);
            return result;
        } else if constexpr (result_always_good_v<result_t>) {
            return pure_result {state, std::move(result).get_value()};
        } else {
            if (result) {
                return noam::result<value_t> {
                    state, std::move(result).get_value()};
            } else {
                return noam::result<value_t> {};
            }
        }
    } / make_parser;
}

/**
 * @brief Takes a Parser a and returns Parser Maybe a (which will always
 * succeed, but the value may be nullopt)
 *
 * @param parser the parser being given to the combinator to be transformed
 */
template <class Parser>
constexpr auto try_parse(Parser&& parser) {
    return [parser = std::forward<Parser>(parser)](state_t state) {
        auto result = parser.parse(state);
        bool result_good = result;
        return pure_result {
            result_good ? result.get_state() : state,
            result_good ? std::optional {std::move(result).get_value()}
                        : std::nullopt};
    } / make_parser;
}

/**
 * @brief Attempts to parse a value, but does so with lookahead (so no part
 * of the string is consumed)
 *
 * @param parser the parser being given to the combinator to be transformed
 */
template <class Parser>
constexpr auto try_lookahead(Parser&& parser) {
    return [parser = std::forward<Parser>(parser)](state_t state) {
        auto result = parser.parse(state);
        return pure_result {
            state, // Because we're doing lookahead, state doesn't get
                   // updated
            result ? std::optional {std::move(result).get_value()}
                   : std::nullopt};
    } / make_parser;
}
} // namespace noam
