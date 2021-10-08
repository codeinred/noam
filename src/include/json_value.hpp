#pragma once
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <map>
#include <noam/util/overload_set.hpp>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <rva/variant.hpp>

namespace json {
constexpr std::nullptr_t null = nullptr;

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
} // namespace json

template <class... Args>
struct fmt::formatter<std::variant<Args...>> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();
        while (it != end && *it != '}')
            it++;
        return it;
    }
    template <class Ctx>
    auto format(std::variant<Args...> const& v, Ctx& ctx)
        -> decltype(ctx.out()) {
        if (v.valueless_by_exception()) {
            return format_to(ctx.out(), "[Valueless]\n");
        } else {
            return std::visit(
                [&](auto& val) -> decltype(ctx.out()) {
                    return format_to(ctx.out(), "{}", val);
                },
                v);
        }
    }
};
template <>
struct fmt::formatter<json::json_value> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();
        while (it != end && *it != '}')
            it++;
        return it;
    }
    template <class Ctx>
    auto format(std::string& depth, json::json_value const& v, Ctx& ctx)
        -> decltype(ctx.out()) {
        auto callable = noam::overload_set {
            [&](auto const& val) { format_to(ctx.out(), "{}", val); },
            [&](json::null_type) { format_to(ctx.out(), "null"); },
            [&](std::string_view sv) { format_to(ctx.out(), "\"{}\"", sv); },
            [&](json::array const& arr) {
                auto begin = arr.begin();
                auto end = arr.end();
                if (begin == end) {
                    format_to(ctx.out(), "[]");
                } else {
                    format_to(ctx.out(), "[");
                    for (;;) {
                        format(depth, *begin, ctx);
                        ++begin;
                        if (begin == end) {
                            format_to(ctx.out(), "]");
                            break;
                        }
                        format_to(ctx.out(), ", ");
                    }
                }
            },
            [&](json::object const& obj) {
                auto begin = obj.begin();
                auto end = obj.end();
                if (begin == end) {
                    format_to(ctx.out(), "[]");
                } else {
                    format_to(ctx.out(), "{}\n", '{');
                    depth += "    ";
                    for (;;) {
                        auto const& [key, val] = *begin;
                        format_to(ctx.out(), "{}\"{}\": ", depth, key);
                        format(depth, val, ctx);
                        ++begin;
                        if (begin == end) {
                            break;
                        }
                        format_to(ctx.out(), ",\n");
                    }
                    if (depth.size() >= 4)
                        depth.resize(depth.size() - 4);
                    format_to(ctx.out(), "\n{}{}", depth, '}');
                }
            }};
        if (v.valueless_by_exception()) {
            return format_to(ctx.out(), "[Valueless]\n");
        } else {

            rva::visit(callable, v);
            return ctx.out();
        }
    }
    template <class Ctx>
    auto format(json::json_value const& v, Ctx& ctx) -> decltype(ctx.out()) {
        std::string depth;
        return format(depth, v, ctx);
    }
};
