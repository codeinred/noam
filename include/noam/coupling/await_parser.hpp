#pragma once
#include <noam/type_traits.hpp>
#include <noam/util/stdlib_coroutine.hpp>
#include <utility>

namespace noam {
template <class Parser>
struct await_parser {
    Parser parser {};
    // This is provided by the promise type via await_transform
    // It's non-owned so using a pointer here is fine
    state_t* state = nullptr;

    using result_t = std::decay_t<decltype(std::declval<Parser>().parse(state_t {}))>;
    result_t result;
    // Gets a copy of the internal state
    constexpr state_t copy_state() const noexcept { return *state; }
    constexpr bool await_ready() noexcept {
        result = std::forward<Parser>(parser).parse(copy_state());
        if (result) {
            // We only update the state if the parse succeeded
            *state = result.get_state();
            return true;
        } else {
            // Don't update the state
            return false;
        }
    }
    // If the result wasn't good, we're shutting the whole thing down.
    // await_suspend is suspending the whole-ass m'fucking operation
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    constexpr decltype(auto) await_resume() & noexcept {
        return std::forward<decltype(result)>(result).get_value();
    }
    constexpr decltype(auto) await_resume() const& noexcept {
        return result.get_value();
    }
    constexpr auto await_resume() && noexcept {
        return std::move(*this).result.get_value();
    }
};

// template <always_good_parser Parser>
// struct await_parser<Parser> {
//     Parser parser {};
//     state_t* state = nullptr;
//     constexpr state_t copy_state() const noexcept { return *state; }
// 
//     constexpr bool await_ready() const noexcept { return true; }
//     constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
// 
//     constexpr auto await_resume() {
//         auto result = std::move(parser).parse(copy_state());
//         *state = result.get_state();
//         return std::move(result).get_value();
//     }
// };

template <class Parser>
await_parser(Parser, state_t*) -> await_parser<Parser>;
} // namespace noam
