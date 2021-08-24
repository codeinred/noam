#include <iostream>
#include <noam/co_await.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <string>
#include <vector>

using std::string_literals::operator""s;

using noam::match_ch;
using noam::parse_double;
using noam::parser;
using noam::pure;
using noam::surround;
using noam::ws;

// This is a placeholder. Replace with actual value.
parser value = parse_double;
parser element = surround(ws, value, ws);
parser string = pure("place holder"s);
// NB: it should be parsing a std::pair. To-do: add std::pair combinator
parser member = surround(ws, string, ws) >> (match_ch<':'> >>  element);
int main() {}
