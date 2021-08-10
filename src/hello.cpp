#include <iostream>
#include <noam/co_parse.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <vector>

constexpr auto read_int = []() -> noam::co_parse<long> {
    co_return co_await noam::parse_long;
};

int main() {
    std::string_view str = "93289 hello";
    auto result = read_int().parse(str);
    std::cout << "Value: " << result.get_value() << '\n';
    std::cout << "Remaining: '" << result.get_state() << "'\n";
}
