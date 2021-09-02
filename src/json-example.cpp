#include "include/json_value.hpp"
#include <iostream>
#include <map>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <noam/util/fmt.hpp>

namespace json {
using noam::parser;
auto parse_value(noam::state_t st) -> noam::result<json_value> {
    constexpr parser value = noam::parser {parse_value};

    constexpr parser member = noam::make<std::pair>(
        noam::parse_string_view, noam::join(noam::separator<':'>, value));

    constexpr parser p = noam::either<json_value>(
        noam::literal_constant<"null", null>, // Parses "null" as json::null
        noam::parse_bool,
        noam::parse_double,
        noam::parse_string_view,
        noam::sequence<'{', '}'>(member), // Parse a json object
        noam::sequence<'[', ']'>(value)); // Parse a json array
    return p.parse(st);
}

constexpr parser parse_json = whitespace_enclose(parser {parse_value});
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
						"GlossSeeAlso": ["GML", "XML"]
                    },
					"GlossSee": "markup"
                }
            }
        }
    }
})";

int main() {
    constexpr auto int_seq =
        noam::sequence(noam::parse_int, noam::separator<','>);
    constexpr auto int_array =
        noam::enclose(noam::separator<'['>, int_seq, noam::separator<']'>);

    fmt::print(
        "{}\n",
        int_seq.parse("10, 20, 30").check_value(std::vector {10, 20, 30}));
    fmt::print("{}\n", int_seq.parse("10").check_value(std::vector {10}));
    fmt::print("{}\n", int_seq.parse("").check_value(std::vector<int> {}));
    fmt::print(
        "{}\n",
        int_array.parse("[10, 20, 30]").check_value(std::vector {10, 20, 30}));
    using json::parse_json;
    fmt::print("{}\n", parse_json.parse("10").check_value(10.0));
    fmt::print("{}\n", parse_json.parse("   10   ").check_value(10.0));
    fmt::print(
        "{}\n", parse_json.parse("null").check_value(json::null_type {}));
    fmt::print(
        "{}\n",
        parse_json.parse(R"(  "hello world"   )")
            .check_value(json::string("hello world")));
    fmt::print("{}\n", parse_json.parse("{}").check_value(json::object {}));
    fmt::print("{}\n", parse_json.parse("[]").check_value(json::array {}));
    fmt::print(
        "{}\n",
        parse_json.parse("[null, null]")
            .check_value(json::array {json::null_type {}, json::null_type {}}));
    fmt::print("{}\n", parse_json.parse(input).good());
}
