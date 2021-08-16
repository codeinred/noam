#include <benchmark/benchmark.h>

#include <algorithm>
#include <noam/co_result.hpp>
#include <noam/combinators.hpp>
#include <noam/intrinsics.hpp>
#include <random>
#include <vector>

constexpr noam::state_t sequence_input =
    "7014, 12696, 29568, 28176, 23527, 22429, 3028, 26971, 1416, 24006, 17784, "
    "3877, 27521, 26757, 23749, 11086, 4652, 970, 2669, 12260, 12797, 21246, "
    "20196, 17407, 6421, 29361, 6382, 16879, 9289, 31432, 8213, 16303, 11360, "
    "5013, 11711, 2120, 27442, 14740, 29091, 28858, 5978, 14107, 32735, 731, "
    "8097, 23716, 11817, 12749, 24686, 14487, 25010, 4716, 2965, 12438, 22123, "
    "9386, 9031, 28505, 26265, 18320, 27170, 1710, 1855, 5762, 6723, 13566, "
    "7882, 1397, 28306, 4205, 30255, 1516, 18313, 30222, 2248, 26410, 21171, "
    "14065, 6391, 13089, 28552, 31401, 17805, 31517, 11071, 7161, 8135, 20102, "
    "2898, 1632, 5654, 30068, 3342, 7509, 3063, 10065, 21076, 10945, 11462, "
    "16614, 15151, 8949, 18131, 696, 6404, 20379, 27106, 27575, 1676, 729, "
    "7896, 30229, 32131, 25702, 28978, 10434, 95, 4346, 30537, 2993, 5978, "
    "3423, 294, 9321, 10933, 3357, 19386, 32009, 14302, 30849, 15855, 29453, "
    "7030, 1218, 30149, 13434, 21597, 24487, 8241, 23274, 25217, 16138, 20735, "
    "24580, 9072, 16945, 2246, 9167, 21291, 15, 12160, 27270, 3439, 12454, "
    "3823, 14372, 15811, 23209, 13613, 30114, 21290, 29468, 26799, 28321, "
    "30687, 24181, 8987, 19516, 15900, 17229, 10022, 8349, 599, 30757, 161, "
    "9671, 14935, 2408, 18838, 3458, 2423, 30998, 30728, 5862, 10685, 1783, "
    "20234, 26496, 24993, 1079, 23842, 13515, 30548, 17874, 9068, 28467, 9287, "
    "18056, 15215, 25187, 2517, 25238, 769, 3116, 23227, 930, 12787, 5394, "
    "3338, 31625, 8853, 5762, 29855, 6813, 11624, 7772, 8597, 31859, 1501, "
    "822, 170, 25343, 14337, 30718, 10449, 23406, 26417, 19736, 8694, 8865, "
    "12156, 11211, 1335, 12925, 14327, 24562, 13855, 27114, 29957, 17194, "
    "25971, 6042, 22956, 23058, 12855, 1812, 30831, 21452, 903, 32332, 22274, "
    "1074, 24907, 3844, 31792, 2589, 27250, 25442, 22325, 3176, 1539, 1713, "
    "14387, 2874, 14638, 28714, 27436, 28494, 23060, 24625, 12920, 16263, "
    "30667, 3108, 6553, 10755, 4920, 4616, 32207, 5824, 4180, 21714, 6898, "
    "29088, 25558, 5922, 31677, 20040, 31364, 21234, 23216, 135, 22948, 4835, "
    "3009, 4818, 781, 30446, 544, 23841, 22303, 13464, 7336, 20203, 16572, "
    "13889, 30958, 21493, 18506, 30397, 27317, 22686, 19343, 1447, 19006, "
    "12133, 7369, 17915, 32173, 5966, 6382, 22621, 6101, 29330, 27456, 9111, "
    "1380, 28237, 6789, 1925, 19310, 29092, 15389, 26646, 16527, 31962, 7768, "
    "14717, 20687, 26274, 12347, 15236, 16192, 31690, 16683, 2431, 11056, "
    "24052, 20346, 10461, 30018, 26728, 315, 3352, 23290, 27771, 12463, 24671, "
    "23241, 19252, 26596, 9783, 15576, 9217, 3662, 32104, 8411, 11430, 14053, "
    "29098, 4936, 26400, 11566, 21128, 25323, 28249, 23559, 3611, 19534, "
    "11138, 14072, 16784, 5098, 14387, 20136, 28389, 9391, 32599, 20292, "
    "32632, 19083, 14120, 9647, 1892, 23337, 13309, 1228, 31749, 24739, 15281, "
    "28079, 29675, 8914, 6878, 18036, 1469, 2359, 8827, 5080, 21893, 19965, "
    "19152, 5910, 25064, 772, 26046, 20685, 10163, 25878, 8209, 10027, 12193, "
    "22329, 19674, 14085, 12898, 216, 15313, 11879, 24955, 30595, 7191, 21863, "
    "6741, 14069, 7131, 8210, 16428, 15958, 13290, 5554, 3156, 32442, 11464, "
    "28220, 446, 4742, 16137, 10609, 30620, 24346, 20636, 10046, 13907, 7543, "
    "24131, 26805, 7759, 6677, 5917, 32714, 4504, 13108, 21809, 11245, 27177, "
    "28940, 19455, 10837, 12131, 32745, 16391, 15287, 32419, 27855, 10739, 98, "
    "32598, 26876, 10707, 30450, 18454, 31344, 7728, 32361, 6119, 31860, "
    "26398, 13878, 5769, 32315, 13824, 10273, 12655, 2866, 21518, 7064, 31806, "
    "8205, 17902, 11169, 8182, 1525, 26456, 7833, 29381, 4427, 7931, 29211, "
    "31303, 18639, 26893, 16989, 17215, 1854, 16582, 23334, 946, 10213, 4444, "
    "6715, 9760, 18268, 16988, 22416, 21134, 5738, 29480, 20173, 13943, 14614, "
    "31342, 22125, 16140, 25031, 29958, 12753, 29458, 5122, 9196, 27994, "
    "23761, 3321, 12215, 8208, 5175, 28798, 31542, 6121, 6243, 3218, 12836, "
    "16003, 21486, 29824, 5651, 9853, 2794, 2364, 30026, 16737, 16978, 28600, "
    "6094, 350, 20863, 3285, 13103, 17554, 8407, 22299, 12780, 32168, 25621, "
    "24995, 7608, 30796, 21025, 6382, 4150, 27268, 9600, 16986, 10504, 31086, "
    "14043, 16155, 8171, 16837, 18519, 5429, 807, 2730, 1262, 6901, 3080, "
    "22125, 10186, 16184, 6911, 18593, 5715, 19691, 17993, 31336, 11919, "
    "25601, 29365, 176, 31983, 747, 27445, 8815, 17733, 5181, 7134, 31776, "
    "21336, 15305, 15846, 7088, 20735, 16653, 9818, 21997, 23554, 12898, "
    "11354, 973, 29082, 18266, 19566, 2030, 5189, 4792, 598, 17108, 30393, "
    "29963, 17285, 29609, 30710, 11962, 5656, 15676, 17143, 12790, 14684, "
    "5711, 28096, 30530, 12799, 16063, 14415, 22617, 5292, 5202, 2748, 16646, "
    "6175, 31830, 2144, 25741, 1092, 7334, 30533, 1691, 24442, 28159, 31654, "
    "8959, 25000, 29597, 20921, 30656, 12505, 5296, 10679, 27189, 11008, 6007, "
    "24952, 23807, 22070, 6599, 13657, 27362, 11801, 16405, 11240, 17976, "
    "15467, 13385, 10950, 16560, 20719, 8715, 18251, 12393, 4106, 17137, "
    "21353, 29106, 13966, 9506, 26995, 26471, 14803, 4906, 20893, 25811, "
    "10913, 13077, 16850, 215, 19676, 30507, 27577, 31478, 14144, 6049, 16686, "
    "29612, 19434, 27636, 13404, 7385, 3584, 31655, 19779, 7690, 16024, 8364, "
    "4029, 29991, 17870, 31024, 23694, 32673, 3162, 11819, 25716, 14075, "
    "24896, 9799, 14290, 11805, 7538, 9099, 10515, 21683, 15148, 27201, 18527, "
    "1815, 22070, 31931, 9200, 25654, 30818, 28979, 576, 14074, 4575, 4605, "
    "11297, 22446, 2861, 2224, 22351, 6023, 14043, 15300, 20098, 6172, 25099, "
    "1620, 17977, 32637, 10719, 28492, 21552, 25868, 22925, 7311, 27683, "
    "12227, 6474, 4115, 5113, 4524, 327, 5690, 18599, 4902, 10295, 29896, "
    "27348, 13157, 32120, 16932, 19180, 13396, 32232, 6511, 19568, 24563, "
    "8131, 4777, 24432, 18851, 501, 13217, 11951, 23426, 20528, 6866, 2886, "
    "27003, 10981, 7999, 31527, 11308, 13689, 17358, 16211, 23985, 14487, "
    "10791, 4374, 13839, 27723, 23554, 27235, 27187, 30065, 14035, 18982, "
    "5429, 18812, 10647, 24280, 19313, 23864, 3463, 9972, 11624, 10329, 12858, "
    "5859, 21310, 20857, 4619, 32619, 1779, 21977, 16062, 25764, 3696, 26853, "
    "30138, 17536, 21809, 20924, 12003, 16228, 18222, 26039, 2443, 23651, "
    "12083, 13090, 15163, 31397, 4186, 18626, 8601, 15810, 28955, 21459, "
    "21670, 17497, 9548, 26289, 17348, 11327, 15498, 642, 4323, 19195, 27496, "
    "1693, 3963, 16537, 22618, 15966, 32765, 8072, 9237, 2440, 31723, 21321, "
    "15530, 14118, 19950, 19716, 32744, 28551, 2759, 28931, 17242, 24429, "
    "13660, 26790, 17950, 31009, 5350, 680, 31651, 9673, 19875, 26379, 11367, "
    "23838, 10148, 1217, 7037, 10146, 9289, 16274, 12586, 8244, 4827, 28117, "
    "22362, 24777, 15065, 22338, 20560, 17824, 18501, 5034, 9485, 32161, "
    "31825, 27435, 30402, 4407, 28116, 29286, 14080, 15223, 22897, 25447, "
    "6294, 278, 26664, 13331, 10424, 3185, 29605, 23010, 11429, 1665, 18359, "
    "1023, 26442, 657, 23361, 14235, 18481, 9094, 19269, 27967, 8488, 18326, "
    "22634, 6122, 22733, 17982, 2640, 4046, 438, 25538, 29493, 6732, 25816, "
    "23390, 20063, 3472, 26575, 16900, 26482, 5237, 18565, 12074, 6260, 12240, "
    "12731, 29622, hello world";

constexpr noam::parser add_w_fold = noam::fold_left(
    // parse the first value as a long, so that the result's value is a long
    noam::parse_long,
    // parse the rest of the values as ints
    noam::parse_comma_separator >> noam::parse_int,
    // Add stuff up
    [](long sum, int value) { return sum + value; });

constexpr noam::parser add_w_test_then =
    [](noam::state_t) -> noam::co_result<long> {
    using noam::parse_int;
    using noam::parser;
    using noam::parse_comma_separator;
    using noam::try_parse;

    // Get the first value
    long sum = co_await parse_int;

    // (>>) : Parser a -> Parser b -> Parser b
    // noam::try_parse : Parser a -> Parser Maybe a
    // NB: parse_comma_separator reads whitespace on either side of the comma
    // so (whitespace >> parse_constexpr_prefix<','> >> whitespace)
    //      === parse_separator<','>
    //      === parse_comma_separator
    parser next_value = test_then(
        parse_comma_separator >> parse_int, [&](int value) { sum += value; });

    // While there's a value followed by a comma, add it to the vector
    while (co_await next_value)
        ;

    // Return the vector of values
    co_return sum;
} / noam::make_parser;

constexpr noam::parser add_w_try_parse =
    [](noam::state_t) -> noam::co_result<long> {
    using noam::parse_int;
    using noam::parser;
    using noam::parse_comma_separator;
    using noam::try_parse;

    // Get the first value
    long sum = co_await parse_int;

    // (>>) : Parser a -> Parser b -> Parser b
    // noam::try_parse : Parser a -> Parser Maybe a
    // NB: parse_comma_separator reads whitespace on either side of the comma
    // so (whitespace >> parse_constexpr_prefix<','> >> whitespace)
    //      === parse_separator<','>
    //      === parse_comma_separator
    parser next_value = noam::try_parse(parse_comma_separator >> parse_int);

    // While there's a value followed by a comma, add it to the vector
    while (std::optional value = co_await next_value) {
        sum += *value;
    }

    // Return the vector of values
    co_return sum;
} / noam::make_parser;

constexpr noam::parser add_w_baseline =
    [](noam::state_t sv) -> noam::standard_result<long> {
    // Value used to store from_chars output
    int value = 0;
    // Result of std::from_chars
    auto result = std::from_chars_result {};

    // the start of the string_view givet to std::result
    const char* parse_result_start = sv.data();
    const char* begin = sv.data();
    const char* end = sv.data() + sv.size();

    result = std::from_chars(begin, end, value);
    if (result.ec != std::errc())
        return {};
    // Set sum to be value initially
    long sum = value;

    // Update begin
    begin = result.ptr;
    parse_result_start = result.ptr;
    for (;;) {
        // Read whitespace
        while (begin < end && *begin == ' ')
            begin++;
        // Read comma
        if (begin != end && *begin == ',')
            begin++;
        else
            break;
        // Read whitespace
        while (begin < end && *begin == ' ')
            begin++;

        // Read value
        result = std::from_chars(begin, end, value);
        if (result.ec != std::errc())
            break;

        sum += value;
        begin = result.ptr;
        parse_result_start = result.ptr;
    }
    return {noam::state_t {parse_result_start, end}, sum};
} / noam::make_parser;

template <class Value>
struct parser_test {
    noam::state_t input;
    noam::state_t expected_remainder;
    Value expected_value;
    size_t items = 0;
    noam::state_t get_input() const { return input; }
    /**
     * @brief Return the number of items parsed by this test. If this metric
     * isn't valid, returns 0
     *
     * @return size_t
     */
    size_t get_items() { return items; }
    /**
     * @brief Validates that the result returned by the parser represents a
     * successful parse; that the value of the parse was correct; and that the
     * correct remainder is returned
     *
     * @param result The result produced by the parser that needs to be
     * validated
     */
    void validate(auto result) const {
        using std::literals::string_literals::operator""s;
        // check to ensure that everything is good
        if (!result.good()) {
            throw std::runtime_error("Parse failed");
        }
        if (result.get_value() != expected_value) {
            throw std::runtime_error(
                "Recieved bad value: "s + std::to_string(result.get_value()));
        }
        if (result.get_state() != expected_remainder) {
            throw std::runtime_error("Failed to read entire input string");
        }
    }
};
template <class Value>
parser_test(noam::state_t, noam::state_t, Value value)
    -> parser_test<Value>;
template <class Value>
parser_test(noam::state_t, noam::state_t, Value value, size_t)
    -> parser_test<Value>;

constexpr parser_test test_add {sequence_input, ", hello world", 15998326, 1000};

void BM_parser(benchmark::State& state, auto parser, auto test) {
    for (auto _ : state) {
        // Validate that the parse result was successful
        // This is done to ensure that benchmarks aren't spurious and represent
        // A correctly parsed result
        // It should be an extremely fast operation compared to parsing itself
        test.validate(parser.parse(test.get_input()));
    }
    state.SetBytesProcessed(test.get_input().size() * state.iterations());
    if (test.get_items()) {
        state.SetItemsProcessed(test.get_items() * state.iterations());
    }
}

BENCHMARK_CAPTURE(BM_parser, add_w_try_parse, add_w_try_parse, test_add);
BENCHMARK_CAPTURE(BM_parser, add_w_test_then, add_w_test_then, test_add);
BENCHMARK_CAPTURE(BM_parser, add_w_fold, add_w_fold, test_add);
BENCHMARK_CAPTURE(BM_parser, add_w_baseline, add_w_baseline, test_add);

BENCHMARK_MAIN();
