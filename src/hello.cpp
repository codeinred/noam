#include <iostream>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <vector>

constexpr noam::parser parse_longs =
    [](noam::state_t) -> noam::result<std::vector<long>> {
    using noam::parse_long;
    using noam::parser;
    using noam::comma_separator;
    using noam::try_parse;

    std::vector<long> vect;

    // Get the first value
    vect.push_back(co_await parse_long);

    // (>>) : Parser a -> Parser b -> Parser b
    // noam::try_parse : Parser a -> Parser Maybe a
    // NB: parse_comma_separator reads whitespace on either side of the comma
    // so (whitespace >> match_ch<','> >> whitespace)
    //      === parse_separator<','>
    //      === parse_comma_separator
    parser next_value = try_parse(comma_separator >> parse_long);

    // While there's a value followed by a comma, add it to the vector
    while (std::optional<long> value = co_await next_value) {
        vect.push_back(*value);
    }

    // Return the vector of values
    co_return vect;
} / noam::make_parser;

int main() {
    noam::state_t str = "1, 2, 3, 4, 5, 10, 20, 30, hello";
    std::cout << "Input: \"" << str << "\"\n";
    auto result = parse_longs.parse(str);
    if (result) {
        std::cout << "Values: ";
        for (auto& v : result.get_value()) {
            std::cout << v << ' ';
        }
        std::cout << '\n';
        std::cout << "Remainder: \"" << result.get_state() << "\"\n";
    }
}
