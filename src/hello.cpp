#include <iostream>
#include <noam/co_parse.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <vector>

constexpr auto read_int = [](noam::state_t) -> noam::co_result<long> {
    co_return co_await noam::parse_long;
} / noam::make_parser;
constexpr auto read_ints =
    [](noam::state_t) -> noam::co_result<std::pair<long, long>> {
    auto i1 = co_await read_int;
    co_await noam::whitespace;
    auto i2 = co_await read_int;
    co_return {i1, i2};
} / noam::make_parser;

int main() {
    std::string_view str = "1234 5678";
    auto [i1, i2] = read_ints.parse(str).get_value();

    std::cout << i1 << ", " << i2 << '\n';
}
