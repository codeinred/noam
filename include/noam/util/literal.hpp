#pragma once
#include <noam/state.hpp>
#include <string_view>

namespace noam {
template <size_t N>
struct string_literal {
    char str[N + 1] {};
    string_literal() = default;
    string_literal(string_literal const&) = default;
    string_literal(string_literal&&) = default;
    constexpr string_literal(char const (&s)[N + 1]) {
        for (int i = 0; i < N; i++) {
            str[i] = s[i];
        }
    }
    constexpr size_t size() const noexcept { return N; }
    constexpr bool check_and_update(state_t& st) const noexcept {
        if (st.starts_with(std::string_view(str, N))) {
            st.remove_prefix(N);
            return true;
        } else {
            return false;
        }
    }
};
template <size_t N>
string_literal(char const (&)[N]) -> string_literal<N - 1>;

struct char_literal {
    char ch {};
    char_literal() = default;
    char_literal(char_literal const&) = default;
    char_literal(char_literal&&) = default;
    constexpr char_literal(char ch) noexcept
      : ch(ch) {}
    constexpr size_t size() const noexcept { return 1; }
    constexpr bool check_and_update(state_t& st) const noexcept {
        if (st.starts_with(ch)) {
            st.remove_prefix(1);
            return true;
        } else {
            return false;
        }
    }
};

struct empty_literal {
    constexpr size_t size() const noexcept { return 0; }
    constexpr bool check_and_update(state_t& st) const noexcept { return true; }
};

template <class Base>
struct any_literal : Base {
    using Base::Base;
    using Base::check_and_update;
    using Base::size;
    any_literal() = default;
    any_literal(any_literal const&) = default;
    any_literal(any_literal&&) = default;
};
any_literal()->any_literal<empty_literal>;
any_literal(empty_literal)->any_literal<empty_literal>;
any_literal(char c)->any_literal<char_literal>;
template <size_t N>
any_literal(char const (&)[N]) -> any_literal<string_literal<N - 1>>;
any_literal(char const (&)[1])->any_literal<empty_literal>;
} // namespace noam
