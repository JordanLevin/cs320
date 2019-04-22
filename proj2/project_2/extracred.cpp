#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <climits>
#include <algorithm>
#include "lrucache.hpp"
#include "utils.hpp"

Result set_associative(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int assoc : {2, 4, 8, 16}) {
        int size = ((512) / 32) / assoc;
        std::vector<std::vector<std::pair<bool, std::uint32_t>>> cache(
            size, std::vector<std::pair<bool, std::uint32_t>>(assoc));
        std::vector<LRU> least_used(size);
        std::pair<int, int> result = std::make_pair(0, 0);

        for (auto p : t.accesses) {
            // Calculate the cache line and the index
            std::uint32_t line = p.first - p.first % 32;
            int index = (p.first % (size * 32)) / 32;

            auto& ways = cache[index];
            auto& lru = least_used[index];
            bool hit = false;
            // look for a hit
            for (int i = 0; i < assoc; ++i) {
                if (ways[i].first && ways[i].second == line) {
                    hit = true;
                    lru.access(i);
                    break;
                }
            }
            if (hit)
                result.first++;
            else if (!hit) {
                // find an empty spot in the cache
                bool filled = false;
                for (int i = 0; i < assoc; ++i) {
                    if (ways[i].first == false) {
                        lru.access(i);
                        ways[i] = std::make_pair(true, line);
                        filled = true;
                        break;
                    }
                }
                // otherwise evict something
                if (!filled) {
                    int evict = lru.getLRU();
                    ways[evict] = std::make_pair(true, line);
                    lru.access(evict);
                }
            }
            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}



Result rrip(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int assoc : {2, 4, 8, 16}) {
        int size = ((512) / 32) / assoc;
        std::vector<std::vector<std::pair<bool, std::uint32_t>>> cache(
            size, std::vector<std::pair<bool, std::uint32_t>>(assoc));
        std::vector<LRU> least_used(size);
        std::vector<std::vector<uint8_t>> gcounters(size, std::vector<uint8_t>(assoc, 3));
        std::pair<int, int> result = std::make_pair(0, 0);

        for (auto p : t.accesses) {
            // Calculate the cache line and the index
            std::uint32_t line = p.first - p.first % 32;
            int index = (p.first % (size * 32)) / 32;

            auto& ways = cache[index];
            auto& counters = gcounters[index];
            bool hit = false;
            // look for a hit
            for (int i = 0; i < assoc; ++i) {
                if (ways[i].first && ways[i].second == line) {
                    hit = true;
                    counters[i] = 0;
                    break;
                }
            }
            if (hit)
                result.first++;
            else if (!hit) {
                // find an empty spot in the cache
                bool filled = false;
                for (int i = 0; i < assoc; ++i) {
                    if (ways[i].first == false) {
                        counters[i] = 0;
                        ways[i] = std::make_pair(true, line);
                        filled = true;
                        break;
                    }
                }
                // otherwise evict something
                if (!filled) {
                    auto evict = std::max_element(counters.begin(), counters.end());
                    ways[evict-counters.begin()] = std::make_pair(true, line);
                    *evict = 0;
                }
            }
            for(uint8_t& i: counters)
                i = std::min(i+1, 3);
            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}



int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "ERROR WRONG ARGS\n";
        exit(0);
    }
    Trace t = read(argv[1]);
    std::string output = argv[2];

    rrip(t).print(output, true);
    set_associative(t).print(output);
}
