#include <cmath>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <noam/intrinsics.hpp>
#include <optional>

bool check(auto x, auto y) { return x == y; }
bool check(float x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-6; }
bool check(double x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-15; }
bool check(long double x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-15; }

enum class color { red, green, blue };

template <noam::parse_result R>
struct fmt::formatter<R> : fmt::formatter<noam::result_value_t<R>> {
    using base = fmt::formatter<noam::result_value_t<R>>;
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    decltype(auto) format(R const& result, FormatContext& ctx) {
        if (result.good()) {
            fmt::format_to(ctx.out(), "[value: ");
            base::format(result.get_value(), ctx);
            return fmt::format_to(ctx.out(), ", state: \"{}\"]", result.get_state());
        } else {
            return fmt::format_to(ctx.out(), "failed");
        }
    }
};

template <>
struct fmt::formatter<noam::state_t> : fmt::formatter<std::string_view> {
    using base = fmt::formatter<std::string_view>;
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    decltype(auto) format(noam::state_t state, FormatContext& ctx) {
        return base::format(state, ctx);
    }
};

bool all_passed = true;
void test(
    noam::state_t name,
    auto&& parser,
    noam::state_t str,
    auto expected,
    noam::state_t remainder) {
    auto result = parser.parse(str);
    bool passed = result.good() && check(result.get_value(), expected)
               && result.get_state() == remainder;
    all_passed = all_passed && passed;
    fmt::print(
        R"(
- name:      "{}"
  input:     "{}"
  expected:  {}
  obtained:  {}
  passed:    {}
)",
        name,
        str,
        noam::pure_result{remainder, expected},
        result,
        passed);
}

#define TEST(parser, str, expected, remainder)                                 \
    test(#parser, parser, str, expected, remainder)
int main() {
    TEST(noam::parse_short, "1234. hello", 1234, ". hello");
    TEST(noam::parse_ushort, "1234. hello", 1234, ". hello");
    TEST(noam::parse_int, "1234. hello", 1234, ". hello");
    TEST(noam::parse_uint, "1234. hello", 1234, ". hello");
    TEST(noam::parse_long, "1234. hello", 1234, ". hello");
    TEST(noam::parse_ulong, "1234. hello", 1234, ". hello");
    TEST(noam::parse_long_long, "1234. hello", 1234, ". hello");
    TEST(noam::parse_ulong_long, "1234. hello", 1234, ". hello");

    TEST(noam::parse_int16, "1234. hello", 1234, ". hello");
    TEST(noam::parse_uint16, "1234. hello", 1234, ". hello");
    TEST(noam::parse_int32, "1234. hello", 1234, ". hello");
    TEST(noam::parse_uint32, "1234. hello", 1234, ". hello");
    TEST(noam::parse_int64, "1234. hello", 1234, ". hello");
    TEST(noam::parse_uint64, "1234. hello", 1234, ". hello");

    TEST(noam::parse_float, "3.14159hello", 3.14159, "hello");
    TEST(noam::parse_float, "3.14159e10hello", 3.14159e10, "hello");
    TEST(noam::parse_double, "3.14159hello", 3.14159, "hello");
    TEST(noam::parse_double, "3.14159e10hello", 3.14159e10, "hello");
    TEST(noam::parse_long_double, "3.14159hello", 3.14159, "hello");
    TEST(noam::parse_long_double, "3.14159e10hello", 3.14159e10, "hello");
    constexpr auto match_1234_prefix = noam::parse_constexpr_prefix<'1', '2', '3', '4'>;
    TEST(match_1234_prefix, "1234. hello", "1234", ". hello");
    return all_passed ? 0 : 1;
}
