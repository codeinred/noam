#pragma once
#include <cmath>
#include <noam/intrinsics.hpp>
#include <noam/util/fmt.hpp>
bool check(auto x, auto y) { return x == y; }
bool check(float x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-6; }
bool check(double x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-15; }
bool check(long double x, auto y) { return x == 0 || std::abs((x - y) / x) < 1e-15; }

bool all_passed = true;
void test(
    noam::state_t name,
    auto&& parser,
    noam::state_t str,
    auto expected,
    noam::state_t remainder) {
    auto result = parser.parse(str);
    bool passed = result && check(result.get_value(), expected)
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
