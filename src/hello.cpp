#include <fmt/ranges.h>
#include <iostream>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <noam/util/fmt.hpp>
#include <vector>

int main() {
    constexpr auto parser = noam::sequence<",">(noam::parse_int);
    noam::state_t str = "10, 20, 30, 40, hello";

    if (auto result = parser.parse(str)) {
        std::cout << "Success" << '\n';
        fmt::print("Success\n");
        fmt::print("State: '{}'\n", result.get_state());
        fmt::print("Value: {}\n", result.get_value());
    } else {
        std::cout << "Failure";
    }
}
