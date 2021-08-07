#include <cstdio>
#include <parser.hpp>

noam::do_parse<int> get_int() {
    co_return 3;
}
int main() {
    printf("Value: %i\n", get_int().run(std::string_view{}).value());
}
