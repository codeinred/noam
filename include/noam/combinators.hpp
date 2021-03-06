#pragma once
#include <noam/intrinsics.hpp>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/type_traits.hpp>
#include <noam/util/combinator_types.hpp>
#include <vector>

namespace noam {
template <class Value>
using pure_parser = parser<parsers::pure<Value>>;
/**
 * @brief Function that takes a value and returns a pure parser that returns
 * that value
 *
 * @param value the value to return when invoking the parser
 */
template <class Value>
constexpr auto pure(Value&& value) {
    return parser {parsers::pure {std::forward<Value>(value)}};
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
    return parsers::map {
        std::forward<Func>(func),
        std::forward<Parser>(parser)};
}

/**
 * @brief base-case for either. Produces a parser that always fails. Idk why
 * you'd need this, but base cases are nice sometimes.
 *
 * @tparam Value
 * @return constexpr auto
 */
template <class Value>
constexpr auto either() {
    return parser {[](state_t st) -> result<Value> { return {}; }};
}
/**
 * @brief Trivial case for `either`. If the value produced by `Parser` is the
 * same as `Value`, then simply returns `p`. Otherwise, returns a parser that
 * creates a pure_result<Value> if p is always good, or a result<Value> if p
 * sometimes fails.
 *
 * @tparam Value
 * @tparam Parser
 * @param p
 * @return constexpr auto
 */
template <class Value, class Parser>
constexpr auto either(Parser&& p) {
    using value_t = parser_value_t<Parser>;
    if constexpr (std::is_same_v<Value, value_t>) {
        return std::forward<Parser>(p);
    } else {
        return parser {[p = std::forward<Parser>(p)](state_t st) {
            if constexpr (parser_always_good_v<Parser>) {
                auto r = p.parse(st);
                return pure_result<Value> {
                    r.get_state(),
                    std::move(r).get_value()};
            } else {
                if (auto r = p.parse(st)) {
                    return result<Value> {
                        r.get_state(),
                        std::move(r).get_value()};
                } else {
                    return result<Value> {};
                }
            }
        }};
    }
}

template <class Value, class PA, class PB>
constexpr auto either(PA&& pa, PB&& pb) {
    constexpr bool good = parser_always_good_v<PA> // <br>
                       || parser_always_good_v<PB>;
    using result_t = get_result_t<Value, good>;
    return parser {
        [pa = std::forward<PA>(pa),
         pb = std::forward<PB>(pb)](state_t st) -> result_t {
            if (auto r = pa.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pb.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if constexpr (!good) {
                return {};
            }
        }};
}
template <class Value, class PA, class PB, class PC>
constexpr auto either(PA&& pa, PB&& pb, PC&& pc) {
    constexpr bool good = parser_always_good_v<PA> // <br>
                       || parser_always_good_v<PB> // <br>
                       || parser_always_good_v<PC>;
    using result_t = get_result_t<Value, good>;
    return parser {
        [pa = std::forward<PA>(pa),
         pb = std::forward<PB>(pb),
         pc = std::forward<PC>(pc)](state_t st) -> result_t {
            if (auto r = pa.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pb.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pc.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if constexpr (!good) {
                return {};
            }
        }};
}
template <class Value, class PA, class PB, class PC, class PD>
constexpr auto either(PA&& pa, PB&& pb, PC&& pc, PD&& pd) {
    constexpr bool good = parser_always_good_v<PA> // <br>
                       || parser_always_good_v<PB> // <br>
                       || parser_always_good_v<PC> // <br>
                       || parser_always_good_v<PD>;
    using result_t = get_result_t<Value, good>;
    return parser {
        [pa = std::forward<PA>(pa),
         pb = std::forward<PB>(pb),
         pc = std::forward<PC>(pc),
         pd = std::forward<PD>(pd)](state_t st) -> result_t {
            if (auto r = pa.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pb.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pc.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if (auto r = pd.parse(st)) {
                return {r.get_state(), std::move(r).get_value()};
            }
            if constexpr (!good) {
                return {};
            }
        }};
}
/**
 * @brief Creates a backtracking parser that will test each parser in sequence
 *
 * @tparam P the types of the parsers to test
 * @param parsers the parsers to test
 * @return parser A parser that will test each input in sequence, returning the
 * first successful result
 */
template <class Value, class... P>
constexpr auto either(P&&... parsers) {
    constexpr bool always_good = (parser_always_good_v<P> || ...);
    using result_t = get_result_t<Value, always_good>;
    return parser {[... p = std::forward<P>(parsers)](state_t st) -> result_t {
        // Non-default-constructible values are boxed so that they're
        // default-constructible.
        box_if_necessary_t<Value> val;
        if constexpr (always_good) {
            (parse_assign_value(st, p, val) || ...);
            return {st, std::move(val)};
        } else {
            if ((parse_assign_value(st, p, val) || ...))
                return {st, std::move(val)};
            else
                return {};
        }
    }};
}

template <class... P>
constexpr auto either(P&&... parsers) {
    using Value = std::common_type_t<parser_value_t<P>...>;
    return either<Value>(std::forward<P>(parsers)...);
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
constexpr auto enclose(Prefix&& pre, Parser&& par, Postfix&& post) {
    return parser {parsers::enclose {
        std::forward<Prefix>(pre),
        std::forward<Parser>(par),
        std::forward<Postfix>(post)}};
}
/**
 * @brief Matches a parser `p` with whitespace surrouding it
 *
 * @tparam Parser
 * @param p
 * @return constexpr auto
 */
template <class Parser>
constexpr auto whitespace_enclose(Parser&& p) {
    return parser {
        parsers::enclose {whitespace, std::forward<Parser>(p), whitespace}};
}

template <class P>
constexpr auto join(P&& parser) {
    return std::forward<P>(parser);
}
/**
 * @brief Applies a series of parsers in sequence. Returns the value produced
 * by the last parser in the sequence. Succeeds iff the last parser in the
 * sequence succeeds.
 *
 * @tparam P
 * @param parsers
 * @return constexpr auto
 */
template <class... P>
constexpr auto join(P&&... parsers) {
    return parser {parsers::join {std::forward<P>(parsers)...}};
}
/**
 * @brief Applies a series of parsers in sequence. Discards whatever values were
 * produced. Succeeds iff the sequence succeeded.
 *
 * @tparam P
 * @param parsers
 * @return constexpr auto
 */
template <class... P>
constexpr auto match(P&&... parsers) {
    return parser {parsers::match {std::forward<P>(parsers)...}};
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
                    state,
                    std::move(result).get_value()};
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

/**
 * @brief Make combinator. Creates a parser that will construct a object of type
 * T using arguments obtained by applying parsers p1...pn in succession
 *
 * @tparam T
 * @tparam P
 * @param parsers
 * @return auto
 */
template <class T, class... P>
constexpr auto make(P&&... parsers) {
    return parser {
        [... parsers = std::forward<P>(parsers)](state_t st) -> result<T> {
            return [&](auto... results) -> result<T> {
                if ((parse_assign(st, parsers, results) && ...)) {
                    return {st, T {std::move(results).get_value()...}};
                } else {
                    return {};
                }
            }(default_constructible_parser_result_t<P> {}...);
        }};
}

template <class T>
constexpr auto make() {
    return parser {[](state_t st) { return pure_result<T> {st, T()}; }};
}
template <class T, class P>
constexpr auto make(P&& p) {
    return parser {[p = std::forward<P>(p)](state_t st) {
        if constexpr (parser_always_good_v<P>) {
            auto res = p.parse(st);
            return pure_result<T> {res.get_state(), T {res.get_value()}};
        } else {
            if (auto res = p.parse(st)) {
                return result<T> {res.get_state(), T {res.get_value()}};
            } else {
                return result<T> {};
            }
        }
    }};
}
/**
 * @brief Make combinator. Creates a parser that will construct a object whose
 * type is deduced by invoking T with arguments obtained by applying parsers
 * p1...pn in succession
 *
 * @tparam T
 * @tparam P
 * @param parsers
 * @return auto
 */
template <template <class...> class T, class... P>
constexpr auto make(P&&... parsers) {
    return parser {[... parsers = std::forward<P>(parsers)](state_t st) {
        return [&](auto... results) {
            return (parse_assign(st, parsers, results) && ...)
                     ? result {st, T {std::move(results).get_value()...}}
                     : null_result;
        }(default_constructible_parser_result_t<P> {}...);
    }};
}

template <template <class...> class T, any_literal sep, class P1, class... P2>
constexpr auto make(P1&& first, P2&&... rest) {
    return make<T>(
        std::forward<P1>(first),
        parsers::join {separator<sep>, std::forward<P2>(rest)}...);
}
template <class T, any_literal sep, class P1, class... P2>
constexpr auto make(P1&& first, P2&&... rest) {
    return make<T>(
        std::forward<P1>(first),
        parsers::join {separator<sep>, std::forward<P2>(rest)}...);
}

/**
 * @brief Parses a sequence of elements, returning the result as a vector
 *
 * @tparam ParseElem
 * @tparam ParseSep
 * @param elem
 * @param sep
 * @return constexpr auto
 */
template <any_literal Sep, class P>
constexpr auto sequence(P&& elem) {
    constexpr int initial_reserve = 16;
    using T = parser_value_t<P>;
    using result_t = pure_result<std::vector<T>>;
    return parser {[elem = std::forward<P>(elem)](state_t st) -> result_t {
        constexpr auto sep = separator<Sep>;
        if (auto first = elem.parse(st)) {
            // Update the state since we obtained the first value
            st = first.get_state();

            std::vector<T> value;
            value.reserve(initial_reserve);
            value.push_back(std::move(first).get_value());
            while (auto sep_result = sep.parse(st)) {
                if (auto next = elem.parse(sep_result.get_state())) {
                    // Update the state since we obtained the next value
                    st = next.get_state();
                    value.push_back(std::move(next).get_value());
                } else {
                    break;
                }
            }
            return {st, std::move(value)};
        } else {
            return {st, std::vector<T>()};
        }
    }};
}
template <
    any_literal Opening,
    any_literal Separator,
    any_literal Closing,
    class P>
constexpr auto sequence(P&& elem) {
    constexpr int initial_reserve = 16;
    using T = parser_value_t<P>;
    using result_t = result<std::vector<T>>;
    return parser {[elem = std::forward<P>(elem)](state_t st) -> result_t {
        constexpr auto open = parsers::match {literal<Opening>, whitespace};
        constexpr auto close = parsers::match {whitespace, literal<Closing>};
        constexpr auto sep = separator<Separator>;
        if (!update_state(open.parse(st), st))
            return null_result;

        std::vector<T> value;
        if (auto first = elem.parse(st)) {
            // Update the state since we obtained the first value
            st = first.get_state();
            value.reserve(initial_reserve);
            value.push_back(std::move(first).get_value());
            while (auto sep_result = sep.parse(st)) {
                if (auto next = elem.parse(sep_result.get_state())) {
                    // Update the state since we obtained the next value
                    st = next.get_state();
                    value.push_back(std::move(next).get_value());
                } else {
                    break;
                }
            }
        }
        return update_state(close.parse(st), st)
                 ? result_t {st, std::move(value)}
                 : null_result;
    }};
}
template <any_literal Opening, any_literal Closing, class P>
constexpr auto sequence(P&& elem) {
    return sequence<Opening, ',', Closing>(std::forward<P>(elem));
}
template <class P>
constexpr auto sequence(P&& elem) {
    return sequence<','>(std::forward<P>(elem));
}
/**
 * @brief Parses a sequence of elements, returning the result as a vector
 *
 * @tparam ParseElem
 * @tparam ParseSep
 * @param elem
 * @param sep
 * @return constexpr auto
 */
template <class ParseElem, class ParseSep>
constexpr auto sequence(ParseElem&& elem, ParseSep&& sep) {
    constexpr int initial_reserve = 16;
    using T = parser_value_t<ParseElem>;
    return parser {
        [elem = std::forward<ParseElem>(elem),
         sep = std::forward<ParseSep>(sep)](
            state_t st) -> pure_result<std::vector<T>> {
            if (auto first = elem.parse(st)) {
                // Update the state since we obtained the first value
                st = first.get_state();

                std::vector<T> value;
                value.reserve(initial_reserve);
                value.push_back(std::move(first).get_value());
                while (auto sep_ = sep.parse(st)) {
                    if (auto next = elem.parse(sep_.get_state())) {
                        // Update the state since we obtained the next value
                        st = next.get_state();
                        value.push_back(std::move(next).get_value());
                    } else {
                        break;
                    }
                }
                return {st, std::move(value)};
            } else {
                return {st, std::vector<T>()};
            }
        }};
}

template <
    class Map,
    any_literal Opening,
    any_literal ElemSeparator,
    any_literal KeyValueSeparator,
    any_literal Closing,
    class K,
    class V>
constexpr auto parse_map(K&& key, V&& val) {
    using KeyT = parser_value_t<K>;
    using ValT = parser_value_t<V>;
    using result_t = result<Map>;
    return parser {
        [elem = noam::make<tuplet::pair, KeyValueSeparator>(
             std::forward<K>(key),
             std::forward<V>(val))](state_t st) -> result_t {
            constexpr auto open = parsers::match {literal<Opening>, whitespace};
            constexpr auto close = parsers::match {
                whitespace,
                literal<Closing>};
            constexpr auto sep = separator<ElemSeparator>;
            if (!update_state(open.parse(st), st))
                return null_result;

            Map map;
            if (auto first = elem.parse(st)) {
                // Update the state since we obtained the first value
                st = first.get_state();
                {
                    auto& [key, val] = first.get_value();
                    map.emplace(std::move(key), std::move(val));
                }
                while (auto sep_result = sep.parse(st)) {
                    if (auto next = elem.parse(sep_result.get_state())) {
                        // Update the state since we obtained the next value
                        st = next.get_state();
                        {
                            auto& [key, val] = next.get_value();
                            map.emplace(std::move(key), std::move(val));
                        }
                    } else {
                        break;
                    }
                }
            }
            return update_state(close.parse(st), st)
                     ? result_t {st, std::move(map)}
                     : null_result;
        }};
}

template <class Map, any_literal Opening, any_literal Closing, class K, class V>
constexpr auto parse_map(K&& key, V&& val) {
    return parse_map<Map, Opening, ',', ':', Closing>(
        std::forward<K>(key),
        std::forward<V>(val));
}
template <class Map, class K, class V>
constexpr auto parse_map(K&& key, V&& val) {
    return parse_map<Map, '{', ',', ':', '}'>(
        std::forward<K>(key),
        std::forward<V>(val));
}
template <class T, class Func>
constexpr auto recurse(Func&& func) {
    return parser {parsers::recurse<T, Func> {func}};
}

template <class T, stateless Func>
constexpr auto recurse(Func&& func) {
    return parser {parsers::recurse_constant<T, std::decay_t<Func> {}> {}};
}
} // namespace noam
