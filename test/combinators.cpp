#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include "test_helpers.hpp"

constexpr noam::parser int_or_42 =
    noam::either(noam::parse_int, noam::pure(42));

static_assert(
    std::same_as<
        noam::parser_result_t<decltype(int_or_42)>,
        noam::pure_result<int>>,
    "Expected int_or_42 to have a result type of "
    "noam::pure_result<int>");
int main() {
    TEST(int_or_42, "1234. hello", 1234, ". hello");
    TEST(int_or_42, "hello", 42, "hello");
    return all_passed ? 0 : 1;
}
