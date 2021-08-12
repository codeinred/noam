#include <iostream>
#include <noam/co_result.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <vector>

constexpr noam::parser parse_longs =
    [](noam::state_t) -> noam::co_result<std::vector<long>> {
    std::vector<long> vect;

    // Get the first value
    vect.push_back(co_await noam::parse_long);

    // (>>) : Parser a -> Parser b -> Parser b
    // noam::try_parse : Parser a -> Parser Maybe a
    noam::parser next_value =
        noam::try_parse(noam::parse_comma_separator >> noam::parse_long);

    // While there's a value followed by a comma, add it to the vector
    while (std::optional<long> value = co_await next_value) {
        vect.push_back(*value);
    }

    // Return the vector of values
    co_return vect;
} / noam::make_parser;

int main() {
    std::string_view str = "1, 2, 3, 4, 5, 10, 20, 30, hello";
    std::cout << "Input: \"" << str << "\"\n";
    auto result = parse_longs.parse(str);
    if (result.good()) {
        std::cout << "Values: ";
        for (auto& v : result.get_value()) {
            std::cout << v << ' ';
        }
        std::cout << '\n';
        std::cout << "Remainder: \"" << result.get_state() << "\"\n";
    }
}
