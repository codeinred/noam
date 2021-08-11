#include <iostream>
#include <noam/co_parse.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <vector>

constexpr auto read_int = []() -> noam::co_parse<long> {
    co_return co_await noam::parse_long;
};
constexpr auto read_ints = []() -> noam::co_parse<std::pair<long, long>> {
    noam::parser ri = read_int();
    auto i1 = co_await ri;
    co_await noam::whitespace;
    auto i2 = co_await ri;
    co_return {i1, i2};
};

int main() {
    std::string_view str = "1234 5678";
    auto [i1, i2] = read_ints().parse(str).get_value();

    std::cout << i1 << ", " << i2 << '\n';
}
