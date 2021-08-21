#include "test_helpers.hpp"
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>

constexpr noam::parser int_or_42 =
    noam::either(noam::parse_int, noam::pure(42));
constexpr noam::parser ws_int_ws =
    noam::surround(noam::whitespace, noam::parse_int, noam::whitespace);

static_assert(
    std::same_as<
        noam::parser_result_t<decltype(int_or_42)>,
        noam::pure_result<int>>,
    "Expected int_or_42 to have a result type of "
    "noam::pure_result<int>");
int main() {
    TEST(int_or_42, "1234. hello", 1234, ". hello");
    TEST(int_or_42, "hello", 42, "hello");
    TEST(ws_int_ws, "    \t\t\r\n\t  32938\t\n\r\r\n   hewwo", 32938, "hewwo");
    return all_passed ? 0 : 1;
}
