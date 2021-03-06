#include <iostream>

#include "utils/util.hpp"
#include "vectors/hash_compact_vector.hpp"
#include "../external/essentials/include/essentials.hpp"
#include "../external/cmd_line_parser/include/parser.hpp"

using namespace tongrams;

template <typename HashType>
void perf_test(uint64_t n, uint64_t w) {
    typedef HashType hash_t;
    std::vector<hash_t> keys;
    std::vector<uint64_t> values;
    keys.reserve(n);
    values.reserve(n);

    typename hash_compact_vector<hash_t>::builder hcvb(n, w);
    essentials::uniform_int_rng<uint64_t> distr(0, std::pow(2, w - 1),
                                                essentials::get_random_seed());
    essentials::logger("Generating random (key, value) pairs");
    for (uint64_t i = 0; i < n; ++i) {
        hash_t k = distr.gen();
        uint64_t v = distr.gen();
        keys.push_back(k);
        values.push_back(v);
        hcvb.set(i, k, v);
    }

    hash_compact_vector<hash_t> cv(hcvb);
    essentials::logger("Checking hash_compact_vector random access");
    for (uint64_t i = 0; i < n; ++i) {
        auto got = cv[i];
        util::check(i, got.first, keys[i], "key");
        util::check(i, got.second, values[i], "value");
    }
    essentials::logger("OK");

    essentials::logger("Writing to disk");
    util::save(global::null_header, cv, "./tmp.out");

    essentials::logger("Loading from disk");
    hash_compact_vector<hash_t> loaded;
    size_t file_size = util::load(loaded, "./tmp.out");
    essentials::logger("read " + std::to_string(file_size) + " bytes");

    essentials::logger("Checking loaded values");
    for (uint64_t i = 0; i < n; ++i) {
        auto got = loaded[i];
        util::check(i, got.first, keys[i], "key");
        util::check(i, got.second, values[i], "value");
    }
    essentials::logger("OK");
    std::remove("./tmp.out");
}

int main(int argc, char** argv) {
    using namespace tongrams;
    cmd_line_parser::parser parser(argc, argv);
    parser.add("num_of_values", "Number of values.");
    parser.add("bits_per_hash",
               "Bits per hash key. It must be either 32 or 64.");
    parser.add("bits_per_value", "Bits per value.");
    if (!parser.parse()) return 1;

    uint64_t n = parser.get<uint64_t>("num_of_values");
    if (!n) {
        std::cerr << "Error: number of values must be non-zero." << std::endl;
        std::terminate();
    }

    uint64_t hash_width = parser.get<uint64_t>("bits_per_hash");
    uint64_t w = parser.get<uint64_t>("bits_per_value");

    if (hash_width == 32) {
        perf_test<uint32_t>(n, w);
    } else if (hash_width == 64) {
        perf_test<uint64_t>(n, w);
    } else {
        std::cerr << "Error: hash width must be 32 or 64." << std::endl;
        std::terminate();
    }

    return 0;
}
