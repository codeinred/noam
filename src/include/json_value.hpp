#pragma once
#include <string_view>
#include <variant>
#include <vector>
#include <tuple>

namespace json {
using string = std::string_view;
using number = double;
using boolean = bool;
using null_type = std::nullptr_t;
struct json_value;
using object = std::vector<std::pair<std::string_view, json_value>>;
using array = std::vector<json_value>;
constexpr std::nullptr_t null = nullptr;
struct json_value
  : std::variant<null_type, string, number, boolean, object, array> {
    using base =
        std::variant<null_type, string, number, boolean, object, array>;
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
} // namespace json
