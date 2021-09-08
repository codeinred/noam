#include <cmath>
#include <noam/util/fmt.hpp>
#include <noam/intrinsics.hpp>
#include <optional>
#include "test_helpers.hpp"

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
    return all_passed ? 0 : 1;
}
