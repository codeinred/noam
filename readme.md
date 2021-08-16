# noam parsing library: Parser Combinators for C++

**Aug 14 2021 NOTE: NOT READY FOR PRODUCTION USE YET**

## Benchmarks

Noam is *fast*. Benchmarks indicate that using high-level parser combinators only
results in a 9% slowdown (0.91x) when compared to a hand-written parser, and we're
continuing to work towards making Noam truly zero-overhead.

![Noam benchmark results](/assets/images/benchmark-screenshot.png)

*This benchmark measured the time taken to parse and total 1000 comma-separated values.
This benchmark was selected because it provides a clean measurment of throughput,
and avoided allocation and other non-deterministic operations that are sources of noise.*

In order to build and run the benchmarks yourself, clone the project and then run
the benchmarking script:

```bash
git clone https://github.com/codeinred/noam && noam/bench.sh
```

This will work out of the box if the default compiler on your system is a recent
version of either GCC or Clang, however if necessary you can specify the compiler
by passing the compiler as a CMake flag:
```bash
git clone https://github.com/codeinred/noam &&
    noam/bench.sh -DCMAKE_CXX_COMPILER=/path/to/your/compiler
```
**NB:** Suitable compiler versions include GCC 10.3 or later; or Clang 10.0.0 or later.

## Directory Structure

***include* directory:** This directory contains header files for the project

***src* directory:** This directory contains main files at top-level

***build.sh*:** Contains a simple build script that invokes CMake. *build.sh*
can be run from any directory, not just the project root.
```bash
#!/bin/bash

# This gets the directory of build.sh
# It should be in the project root
# This allows us to run build.sh from any directory, not just the project root
project_root="$(dirname $0)"

# Create build folder in project root
mkdir -p "$project_root/build"
cd "$project_root/build"

# build project in $project_root/build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .
```

## How it works

Every parser in the library is an instantiation of the parser type. The parser
type is templated on a function that takes a string view, and returns a
`parse_result`.

A type satisfying `parse_result` represents a successful parse if `result.good()`
is true, and a failed parse if `result.good()` is false. If the result is good,
the remaining portion of the string can be obtained via `result.get_state()`, and the
value can be obtained via `result.get_value()`.

Getting the value and getting the state are only well-defined if the result was good.

```cpp
// Type which represents a parser
// You can convert any lambda l: state_t -> parse_result into a parser
// Simply by invoking the constructor via parser{l}
template <class Func>
struct parser {
    // Enables empty base class optimization
    // if Func is empty (i.e, [](){}), parser<Func> is too
    [[no_unique_address]] Func parse;
};
template <class Func>
parser(Func) -> parser<Func>;
```
Noam templates on the function that defines the parser, instead of the type
produced by the parser itself, in order to allow the compiler to optimize and
inline parsers so that the library is near-zero-overhead.

```cpp
/**
 * @brief A concept that matches any result type. Result types must have a
 * good() function, a get_value() function, and a get_state() function.
 *
 * @tparam Result the type to test
 */
template <class Result>
concept parse_result = requires(Result result) {
    { result.good() } -> convertible_to<bool>;
    { result.get_value() };
    { result.get_state() } -> same_as<state_t>;
};
```
If you want to get the result or value produced by the parser,
Noam provides `parser_result_t` (for getting the result type),
and `parser_value_t` (for getting the value type, that is, the type returned by result.get_value()).

### Ensuring idempotence with parsers

Let's look at a simple example:

```cpp
// Template type of parser deduced automatically
// This will be noam::parser<(annonymous lambda)>
constexpr noam::parser parse_int = noam::parser {
    [](noam::state_t) -> noam::co_result<int> {
        // Noam provides this already using <charconv> under the hood.
        // See: https://en.cppreference.com/w/cpp/header/charconv
        // The functions provided in <charconv> are known to be highly performant
        int value = co_await noam::parse_int;
        co_return value;
    }
};
```
Every time `parse_int` is used, it invokes the coroutine with whatever string
you're currently parsing. This is tracked by `noam::state_t`, which is a
high-performance nullable string view implementation. `noam::state_t` is optimized
for the specific use case of parsing, so operations like `state.substr(pos)` are
implemented as a single pointer addition. In addition, making `noam::state_t` nullable
allows result types to forgo storing a bool to indicate failure. Instead, a null state
is indicative of failure.

The constructor to `noam::co_result` accepts the coroutine handle as an input,
and invokes it immediately, then stores the result.

Because `co_result` doesn't store the coroutine handle, it's trivially copyable
 and movable, allowing the result to be passed around efficiently.

Immediate invocation of the coroutine, together with a trivially copyable `co_result`,
ensures idempotence:

```cpp
constexpr noam::parser parse_pair = noam::parser {
    [](noam::state_t) -> noam::co_result<std::pair<int, int>> {
        // noam::parse_separator handles whitespace surrounding a separator
        // So "   {   5    ,    10    }   " is accepted
        // As is "{5,10}"
        co_await noam::parse_separator<'{'>;
        int v1 = co_await parse_int;       // Invokes coroutine to obtain result
        co_await noam::parse_separator<','>;
        int v2 = co_await parse_int;       // Invokes coroutine to obtain result
        co_await noam::parse_separator<'}'>;
        co_return {v1, v2};
    }
};
```
