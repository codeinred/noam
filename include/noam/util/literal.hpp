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
    constexpr bool is_prefix_of(std::string_view sv) const {
        return sv.starts_with(std::string_view(str, N));
    }
    constexpr auto remove_prefix(std::string_view sv) const noexcept
        -> std::string_view {
        sv.remove_prefix(N);
        return sv;
    }
};
template <size_t N>
string_literal(char const (&)[N]) -> string_literal<N - 1>;

struct char_literal {
    char ch {};
    constexpr size_t size() const noexcept { return 1; }
    constexpr bool is_prefix_of(std::string_view sv) const noexcept {
        return sv.starts_with(ch);
    }
    constexpr auto remove_prefix(std::string_view sv) const noexcept
        -> std::string_view {
        sv.remove_prefix(1);
        return sv;
    }
};

struct empty_literal {
    constexpr size_t size() const noexcept { return 0; }
    constexpr bool is_prefix_of(std::string_view sv) {
        return true;
    }
    constexpr auto remove_prefix(std::string_view sv) const noexcept -> std::string_view {
        return sv;
    }
};

template<class Base>
struct any_literal : Base {
    using Base::size;
    using Base::is_prefix_of;
    using Base::remove_prefix;
    using Base::Base;
    any_literal() = default;
    any_literal(any_literal const&) = default;
    any_literal(any_literal&&) = default;
};
any_literal() -> any_literal<empty_literal>;
any_literal(empty_literal) -> any_literal<empty_literal>;
any_literal(char c) -> any_literal<char_literal>;
template <size_t N>
any_literal(char const (&)[N]) -> any_literal<string_literal<N - 1>>;
} // namespace noam
