#pragma once
#if __has_include(<fmt/core.h>)
#include <fmt/core.h>
#include <fmt/format.h>
#include <noam/parser.hpp>
#include <noam/concepts.hpp>
#include <noam/util/reflection.hpp>

template <noam::parse_result R>
struct fmt::formatter<R> : fmt::formatter<noam::result_value_t<R>> {
    using base = fmt::formatter<noam::result_value_t<R>>;
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    decltype(auto) format(R const& result, FormatContext& ctx) {
        if (result) {
            fmt::format_to(ctx.out(), "[value: ");
            base::format(result.get_value(), ctx);
            return fmt::format_to(ctx.out(), ", state: \"{}\"]", result.get_state());
        } else {
            return fmt::format_to(ctx.out(), "failed");
        }
    }
};

template <>
struct fmt::formatter<noam::state_t> : fmt::formatter<std::string_view> {
    using base = fmt::formatter<std::string_view>;
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    decltype(auto) format(noam::state_t state, FormatContext& ctx) {
        return base::format(state, ctx);
    }
};

template <class T>
struct fmt::formatter<noam::parser<T>> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    decltype(auto) format(noam::parser<T> const& parser, FormatContext& ctx) {
        return format_to(ctx.out(), "noam::parser<{}>", noam::name_of_type<T>);
    }
};
#else

// A static_assert is used here b/c it allows raw string literals, while #error
// does not
static_assert(false, R"cmake_msg(

This header requires the C++ fmt library as a dependency.
CMake can download it automatically for you if you add:

    include(FetchContent)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    )
    FetchContent_MakeAvailable(fmt)

    target_link_libraries("Your Target" PRIVATE fmt)

To your CMakeLists.txt, or you can install fmt on your system and linking it
via whichever build system you prefer.
)cmake_msg");

#endif
