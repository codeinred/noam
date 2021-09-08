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

template <char... chars>
struct count_chars {
    constexpr auto parse(state_t state) const noexcept -> pure_result<size_t> {
        size_t size = state.size();
        size_t i = 0;
        for (; i < size; i++) {
            char current = state[i];

            // If one of them matches then we continue testing chars
            if (((chars == current) || ...))
                continue;

            break;
        }
        state.remove_prefix(i);
        return noam::pure_result {state, i};
    }
};

struct bool_parser {
    constexpr auto parse(state_t st) const noexcept -> result<bool> {
        if (st.starts_with("true")) {
            return {st.substr(4), true};
        }
        if (st.starts_with("false")) {
            return {st.substr(5), false};
        }
        return {};
    }
};

struct string_parser {
    auto parse(state_t st) const -> result<std::string> {
        if (st.size() >= 2 && st[0] == '"') {
            std::string str;
            size_t i = 1;
            size_t count = st.size() - 1;
            while (i < count) {
                char c = st[i];
                char val = c;
                if (c == '\\') {
                    char next = st[i + 1];
                    switch (next) {
                        case 'b': val = '\b'; break;
                        case 'f': val = '\f'; break;
                        case 'n': val = '\n'; break;
                        case 'r': val = '\r'; break;
                        case 't': val = '\t'; break;
                        case '\\': val = '\\'; break;
                        case '"': val = '"'; break;
                        // The parser fails on an unrecognized escape code
                        default: return {};
                    }
                    i += 2;
                } else if (c == '"') {
                    break;
                } else {
                    val = c;
                    i += 1;
                }
                str.push_back(val);
            }
            if (st[i] == '"') {
                return {st.substr(i + 1), std::move(str)};
            }
        } // namespace noam::parsers
        return {};
    }
};

/**
 * @brief Parses a segment of characters between a beginning delimiter and an
 * ending delimiter as a string view
 *
 * @tparam begin
 * @tparam end
 * @tparam escape
 */
template <any_literal begin, any_literal end, any_literal escape>
struct view_parser {
    constexpr static size_t min_length = begin.size() + end.size();
    constexpr auto parse(state_t st) const -> result<std::string_view> {
        if (begin.check_and_update(st)) {
            char const* view_begin = st.data();
            while (st.size() > 0) {
                char const* view_end = st.data();
                if (end.check_and_update(st)) [[unlikely]] {
                    return {st, std::string_view(view_begin, view_end)};
                }
                if (escape.check_and_update(st)) [[unlikely]] {
                    end.check_and_update(st);
                } else {
                    st.remove_prefix(1);
                }
            }
        }
        return {};
    }
};
} // namespace noam::parsers
