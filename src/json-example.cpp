#include "include/json_value.hpp"
#include <iostream>
#include <map>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <noam/util/fmt.hpp>

namespace json {
using json_value = rva::variant<
    std::nullptr_t,
    std::string_view,
    double,
    bool,
    std::map<std::string_view, rva::self_t>,
    std::vector<rva::self_t>>;

using string = std::string_view;
using number = double;
using boolean = bool;
using null_type = std::nullptr_t;
using array = std::vector<json_value>;
using object = std::map<std::string_view, json_value>;
using noam::parser;
/**
 * The noam::recurse<T> takes a function f : parser T -> parser T, and produces
 * a parser T. This allows you to create parsers that reference themselves.
 *
 * @brief parse_value parses a json value
 *
 */
constexpr parser parse_value = noam::recurse<json_value>([](auto parse_value) {
    return noam::either<json_value>(
        noam::literal_constant<null, "null">, // Parses "null" as json::null
        noam::parse_bool,
        noam::parse_double,
        noam::parse_string_view,
        noam::sequence<'[', ']'>(parse_value), // Parse a json array
        noam::parse_map<json::object>(         // Parses a map as a json::object
            noam::parse_string_view,           // Get the key for the map
            parse_value                        // get the value for the map
            ));
});

/**
 * @brief This is essentially the same as parse_value, except it will trim any
 * whitespace
 *
 */
constexpr parser parse_json = noam::whitespace_enclose(parse_value);
} // namespace json

std::string_view input = R"({
    "glossary": {
        "foo": null,
        "title": "example glossary",
		"GlossDiv": {
            "title": "S",
			"GlossList": {
                "GlossEntry": {
                    "ID": "SGML",
					"SortAs": "SGML",
					"GlossTerm": "Standard Generalized Markup Language",
					"Acronym": "SGML",
					"Abbrev": "ISO 8879:1986",
					"GlossDef": {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso": ["GML", "XML", 10, 20, 30, 40, null, [1, 2, 3, 4, "hello", {"blarg": [[], [], [[], [[[], ["blarg", {"foo": [10, 20, 10, 1]}]], 20]]]}]]
                    },
					"GlossSee": "markup"
                }
            }
        }
    }
})";


void parse_n_print(std::string_view sv) {
    if (auto res = json::parse_json.parse(sv)) {
        fmt::print("{}\n", res.get_value());
    } else {
        fmt::print("failed");
    }
}

int main() { parse_n_print(input); }
