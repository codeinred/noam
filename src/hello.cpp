#include <iostream>
#include <noam/co_parse.hpp>
#include <noam/combinators.hpp>
#include <vector>

bool is_digit(char c) { return '0' <= c && c <= '9'; }

auto pure = [](auto value) {
    return [=](std::string_view state) {
        return noam::pure_result {state, value};
    };
};

auto get_state = [](std::string_view state) {
    return noam::state_result {state};
};

auto get_char = [](std::string_view state) -> noam::standard_result<char> {
    return state.empty()
           ? noam::standard_result<char> {}
           : noam::standard_result<char> {state.substr(1), state[0]};
};

auto get_digit = [](std::string_view state) -> noam::standard_result<int> {
    if (state.empty()) {
        return {};
    } else {
        char c = state[0];

        return is_digit(c)
               ? noam::standard_result<int> {state.substr(1), c - '0'}
               : noam::standard_result<int> {};
    }
};


noam::co_parse<long> read_int() {
    auto fold_digit = [](long a, long b) { return a * 10 + b; };
    int value = co_await noam::fold_left(get_digit, fold_digit);
    co_return value;
}

int main() {
    std::string_view str = "93289 hello";
    auto result = read_int().parse(str);
    std::cout << "Value: " << result.value() << '\n';
    std::cout << "Remaining: '" << result.new_state() << "'\n";
}
