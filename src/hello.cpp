#include <noam/parser.hpp>
#include <iostream>
#include <vector>

auto pure = [](auto val) {
    struct result {
        decltype(val) value_;
        std::string_view state;
        constexpr bool good() const noexcept {
            return true;
        }
        constexpr std::string_view value() {
            return value_;
        }
        constexpr std::string_view new_state() {
            return state;
        }
        result& operator()(std::string_view state_) {
            state = state_;
        }
    };
    return result{val};
};
auto get_state = [](std::string_view sv) {
    struct result {
        std::string_view state;
        constexpr bool good() const noexcept {
            return true;
        }
        constexpr std::string_view value() {
            return state;
        }
        constexpr std::string_view new_state() {
            return state;
        }
    };
    return result{sv};
};
auto get_char = [](std::string_view sv) {
    struct result {
        std::string_view state;
        constexpr bool good() const noexcept {
            return state.size() > 0;
        }
        constexpr char value() const {
            return state[0];
        }
        constexpr std::string_view new_state() const {
            return state.substr(1);
        }
    };
    return result{sv};
};
bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

auto get_digit = [](std::string_view sv) {
    struct result {
        std::string_view state;
        constexpr bool good() const noexcept {
            return state.size() > 0 && is_digit(state[0]);
        }
        constexpr char value() const {
            return state[0];
        }
        constexpr std::string_view new_state() const {
            return state.substr(1);
        }
    };
    return result{sv};
};

noam::do_parse<std::string_view> read_digits() {
    std::string_view current_state = co_await get_state;
    int digits_len = 0;
    while(co_await noam::test(get_digit)) {
        digits_len++;
    }
    std::cout << digits_len << '\n';
    co_return current_state.substr(0, digits_len);
}
auto split_digits() -> noam::do_parse<std::vector<std::string_view>> {
    std::vector<std::string_view> vect;

    std::cout << "in split_digits, initial state: " << (co_await get_state) << std::endl;
    while(true) {
        auto digits = co_await read_digits;
        std::cout << "read " << digits << '\n';
        vect.push_back(digits);
        std::cout << "vect.size: " << vect.size() << '\n';
        if((co_await get_state).empty())
            break;
    }
    std::cout << "Done" << '\n';
    for(auto v : vect) {
        std::cout << v << '\n';
    }
    co_return vect;
}
int main() {
    std::string_view sv = "123 4 5 7634 232423";
    auto result = split_digits().parse(sv);
    for(auto v : result.value()) {
        std::cout << v << '\n';
    }
}
