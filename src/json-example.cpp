#include <iostream>
#include <map>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <noam/util/fmt.hpp>
#include <variant>
#include <vector>
namespace json {
using string = std::string_view;
using number = double;
using boolean = bool;
using null = noam::empty;
struct json_value;
using object = std::vector<std::pair<std::string_view, json_value>>;
using array = std::vector<json_value>;

struct json_value : std::variant<null, string, number, boolean, object, array> {
    using base = std::variant<null, string, number, boolean, object, array>;
    using base::base;
    using base::operator=;
    using base::index;
    bool operator==(json_value const&) const = default;
};

template <class... Args>
constexpr std::variant<Args...>& unwrap(std::variant<Args...>& v) noexcept {
    return v;
}
template <class... Args>
constexpr std::variant<Args...> const&
unwrap(std::variant<Args...> const& v) noexcept {
    return v;
}
template <class... Args>
constexpr std::variant<Args...>&& unwrap(std::variant<Args...>&& v) noexcept {
    return std::move(v);
}

/**
 * @brief Visits a class derived from std::variant by unwrapping it and then
 * calling std::visit
 *
 * @tparam Callable
 * @tparam Variants
 * @param c the callable function with which to visit the variant
 * @param v the variants to visit
 * @return auto
 */
template <class Callable, class... Variants>
auto visit(Callable&& c, Variants&&... v) {
    return std::visit(
        std::forward<Callable>(c), unwrap(std::forward<Variants>(v))...);
}
/**
 * @brief Visits a class derived from std::variant by unwrapping it and then
 * calling std::visit
 *
 * @tparam Callable
 * @tparam Variants
 * @param c the callable function with which to visit the variant
 * @param v the variants to visit
 * @return auto
 */
template <class R, class Callable, class... Variants>
auto visit(Callable&& c, Variants&&... v) {
    return std::visit<R>(
        std::forward<Callable>(c), unwrap(std::forward<Variants>(v))...);
}
using noam::parser;
using noam::state_t;
auto parse_value_impl(state_t st) -> noam::result<json_value>;
constexpr parser parse_null = (noam::match_constexpr_prefix<'n', 'u', 'l', 'l'>)
                           >> noam::make<null>();
constexpr parser parse_value = parser {parse_value_impl};
constexpr parser parse_array = surround(
    noam::match_separator<'['>,
    sequence(parse_value, noam::match_separator<','>),
    noam::match_separator<']'>);
constexpr parser parse_member = noam::make<std::pair>(
    noam::parse_string_view, (noam::match_separator<':'>) >> parse_value);
constexpr parser parse_object = noam::surround(
    noam::match_separator<'{'>,
    noam::sequence(parse_member, noam::match_separator<','>),
    noam::match_separator<'}'>);
auto parse_value_impl(state_t st) -> noam::result<json_value> {
    constexpr auto p = noam::either<json_value>(
        parse_null,
        noam::parse_bool,
        noam::parse_double,
        noam::parse_string_view,
        parse_object,
        parse_array);
    return p.parse(st);
}
constexpr parser parse_json = surround(noam::ws, parse_value, noam::ws);
} // namespace json

std::string_view input = R"({
    "glossary": {
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

static_assert(noam::match_separator<'['>.parse("   [    ").good());
int main() {
    constexpr auto int_seq =
        noam::sequence(noam::parse_int, noam::match_separator<','>);
    constexpr auto int_array = noam::surround(
        noam::match_separator<'['>, int_seq, noam::match_separator<']'>);

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
    fmt::print("{}\n", parse_json.parse("null").check_value(json::null {}));
    fmt::print(
        "{}\n",
        parse_json.parse(R"(  "hello world"   )")
            .check_value(json::string("hello world")));
    fmt::print("{}\n", parse_json.parse("{}").check_value(json::object {}));
    fmt::print("{}\n", parse_json.parse("[]").check_value(json::array {}));
    fmt::print(
        "{}\n",
        parse_json.parse("[null, null]")
            .check_value(json::array {json::null {}, json::null {}}));
    fmt::print("{}\n", parse_json.parse(input).good());
}
