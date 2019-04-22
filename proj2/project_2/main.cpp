#include <cstdint>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "lrucache.hpp"
#include "utils.hpp"

Result direct_mapped(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int size :
         {1024 / 32, 1024 * 4 / 32, 1024 * 16 / 32, 1024 * 32 / 32}) {
        // issue is size is 32 so everything indexes into 32
        std::vector<std::pair<bool, std::uint32_t>> cache(size);
        std::pair<int, int> result = std::make_pair(0, 0);
        for (auto p : t.accesses) {
            // Calculate the cache line and the index
            std::uint32_t line = p.first - p.first % 32;
            int index = (p.first % (size * 32)) / 32;
            if (cache[index].first && cache[index].second == line)
                result.first++;
            result.second++;

            cache[index] = std::make_pair(true, line);
        }
        ret.res.push_back(result);
    }
    return ret;
}

Result set_associative(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int assoc : {2, 4, 8, 16}) {
        int size = ((1024 * 16) / 32) / assoc;
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

Result fully_associative(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    int assoc = 1024 * 16 / 32;  // associativity is 16kb/line size
    // int assoc = 4;
    std::vector<std::pair<bool, std::uint32_t>> cache(assoc);
    std::vector<bool> hotcold(assoc - 1);
    std::pair<int, int> result = std::make_pair(0, 0);

    for (auto p : t.accesses) {
        // Calculate the cache line and the index
        std::uint32_t line = p.first - p.first % 32;
        int index = -1;
        for (int i = 0; i < cache.size(); i++) {
            if (cache[i].first && cache[i].second == line) {
                result.first++;
                index = i;
                break;
            }
        }
        int start = 0, end = hotcold.size(), hcindex, prev = -1;
        while (true) {
            hcindex = (start + end) / 2;

            if (index == -1) {
                if (hotcold[hcindex])
                    end = hcindex;
                else
                    start = hcindex + 1;
            }

            if (index != -1 && prev != hcindex) {
                if (index > hcindex) {
                    hotcold[hcindex] = 1;
                    start = hcindex + 1;
                } else if (index <= hcindex) {
                    hotcold[hcindex] = 0;
                    end = hcindex;
                }
            }

            if (index == -1 && prev != hcindex) {
                hotcold[hcindex] = !hotcold[hcindex];
            }
            prev = hcindex;
            // Binary search has finished
            if (end - start <= 0) {
                // If there was a cache miss then evict the LRU
                // Otherwise the hotcold bits are updated and we're done
                if (index == -1) {
                    if (hotcold[hcindex] == 1)
                        cache[hcindex + 1] = std::make_pair(true, line);
                    else
                        cache[hcindex] = std::make_pair(true, line);
                }
                break;
            }
        }

        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result fully_set_associative(const Trace& t) {
    Result ret;
    int assoc = 1024 * 16 / 32;  // associativity is 16kb/line size
    int size = ((1024 * 16) / 32) / assoc;
    std::vector<std::pair<bool, std::uint32_t>> cache(assoc);
    std::vector<LRU> least_used(assoc);

    std::vector<std::pair<int, int>> results;
    std::pair<int, int> result = std::make_pair(0, 0);

    for (auto p : t.accesses) {
        // Calculate the cache line and the index
        std::uint32_t line = p.first - p.first % 32;
        int index = (p.first % (size * 32)) / 32;

        auto& lru = least_used[index];
        bool hit = false;
        // look for a hit
        for (int i = 0; i < assoc; ++i) {
            if (cache[i].first && cache[i].second == line) {
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
                if (cache[i].first == false) {
                    lru.access(i);
                    cache[i] = std::make_pair(true, line);
                    filled = true;
                    break;
                }
            }
            // otherwise evict something
            if (!filled) {
                int evict = lru.getLRU();
                cache[evict] = std::make_pair(true, line);
                lru.access(evict);
            }
        }
        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result set_associative_no_alloc(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int assoc : {2, 4, 8, 16}) {
        int size = ((1024 * 16) / 32) / assoc;
        std::vector<std::vector<std::pair<bool, std::uint32_t>>> cache(
            size, std::vector<std::pair<bool, std::uint32_t>>(assoc));
        std::vector<LRU> least_used(size);
        std::pair<int, int> result = std::make_pair(0, 0);

        for (auto p : t.accesses) {
            result.second++;
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
            if(p.second)
                continue;
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
        }
        ret.res.push_back(result);
    }
    return ret;
}


Result set_associative_prefetch(const Trace& t, bool on_miss=false) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for (int assoc : {2, 4, 8, 16}) {
        int size = ((1024 * 16) / 32) / assoc;
        std::vector<std::vector<std::pair<bool, std::uint32_t>>> cache(
            size, std::vector<std::pair<bool, std::uint32_t>>(assoc));
        std::vector<LRU> least_used(size);
        std::pair<int, int> result = std::make_pair(0, 0);

        for (auto p : t.accesses) {
            result.second++;
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
            if (!hit) {
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
            //Code only triggered for prefetch_on_miss
            if(on_miss == true && hit){
                result.first++;
                continue;
            }
            // look for a hit on the prefetch line
            index = ((p.first+32) % (size * 32)) / 32;
            auto& ways_pref = cache[index];
            auto& lru_pref = least_used[index];
            bool hit_pref = false;
            for (int i = 0; i < assoc; ++i) {
                if (ways_pref[i].first && ways_pref[i].second == line+32) {
                    hit_pref = true;
                    lru_pref.access(i);
                    break;
                }
            }
            if (!hit_pref) {
                // find an empty spot in the cache
                bool filled_pref = false;
                for (int i = 0; i < assoc; ++i) {
                    if (ways_pref[i].first == false) {
                        lru_pref.access(i);
                        ways_pref[i] = std::make_pair(true, line+32);
                        filled_pref = true;
                        break;
                    }
                }
                // otherwise evict something
                if (!filled_pref) {
                    int evict = lru_pref.getLRU();
                    ways_pref[evict] = std::make_pair(true, line+32);
                    lru_pref.access(evict);
                }
            }
            if (hit)
                result.first++;
        }
        ret.res.push_back(result);
    }
    return ret;
}

Result prefetch_on_miss(const Trace& t){
    return set_associative_prefetch(t, true);
}



int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "ERROR WRONG ARGS\n";
        exit(0);
    }
    Trace t = read(argv[1]);
    std::string output = argv[2];

    direct_mapped(t).print(output, true);
    set_associative(t).print(output);
    fully_set_associative(t).print(output);
    fully_associative(t).print(output);
    set_associative_no_alloc(t).print(output);
    set_associative_prefetch(t).print(output);
    prefetch_on_miss(t).print(output);
}
