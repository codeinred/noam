#pragma once
#include <charconv>
#include <cstddef>
#include <noam/operators.hpp>
#include <noam/parser.hpp>
#include <noam/result_types.hpp>
#include <noam/util/combinator_types.hpp>
#include <noam/util/literal.hpp>
#include <string>

namespace noam::parsers {
template <class T>
struct charconv {
    auto parse(state_t state) const -> result<T> {
        auto start_ = state.data();
        auto end_ = state.data() + state.size();
        T value;
        auto result = std::from_chars(start_, end_, value);
        if (result.ec == std::errc()) {
            return {state_t(result.ptr, end_), value};
        } else {
            return {};
        }
    }
};

/**
 * @brief Checks if a string is prefixed by any literal in the sequence. If
 * true, removes the prefix. The result is the empty type.
 *
 * @tparam Literals the sequence of literals to match against.
 */
template <any_literal... Literals>
struct literal {
    constexpr auto parse(state_t st) const -> result<empty> {
        if ((Literals.check_and_update(st) || ...)) {
            return {st, empty {}};
        } else {
            return {};
        }
    }
};

/**
 * @brief Checks if a string is prefixed by any literal in the sequence. If
 * true, removes the prefix. The result is a default-constructed T
 *
 * @tparam T the type to return
 * @tparam Literals the sequence of literals to match against.
 */
template <class T, any_literal... Literals>
struct literal_makes {
    constexpr auto parse(state_t st) const -> result<T> {
        if ((Literals.check_and_update(st) || ...)) {
            return {st, T {}};
        } else {
            return {};
        }
    }
};

/**
 * @brief Checks if a string is prefixed by any literal in the sequence. If
 * true, removes the prefix. The result's value is given by `constant`
 *
 * @tparam constant the value to return on a successful parse
 * @tparam Literals the sequence of literals to match against.
 */
template <auto constant, any_literal... Literals>
struct literal_constant {
    using T = std::decay_t<decltype(constant)>;
    constexpr auto parse(state_t st) const -> result<T> {
        if ((Literals.check_and_update(st) || ...)) {
            return {st, constant};
        } else {
            return {};
        }
    }
};

template <char... chars>
struct zero_or_more_chars {
    constexpr auto parse(state_t state) const noexcept -> pure_result<empty> {
        char const* begin = state._begin;
        char const* end = state._end;
        while (begin < end && ((*begin == chars) || ...))
            begin++;
        return {state_t {begin, end}, empty {}};
    }
};
} // namespace noam::parsers
