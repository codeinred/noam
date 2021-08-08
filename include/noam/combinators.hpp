#include <noam/result_types.hpp>

// This file holds functios that transform or modify parsers

namespace noam {
auto fold_left = [](auto parser, auto fold) {
    using value_t = std::decay_t<decltype(parser(std::string_view {}).value())>;
    return [=](std::string_view state) -> noam::standard_result<value_t> {
        auto result = parser(state);
        if (!result.good()) {
            return {};
        }
        state = result.new_state();
        value_t value = result.value();
        for (result = parser(state); result.good(); result = parser(state)) {
            value = fold(value, result.value());
            state = result.new_state();
        }
        return noam::standard_result {state, value};
    };
};

template <class F, any_parser Parser>
auto map(F&& func, Parser&& parser) {
    return [f = std::forward<F>(func),
            p = std::forward<Parser>(parser)](state_t state) {
        // to-do: implement map on parser_result
    };
}

} // namespace noam
