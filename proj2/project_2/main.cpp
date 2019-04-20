#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include "utils.hpp"
#include "lrucache.hpp"

Result direct_mapped(const Trace& t) {
    Result ret;
    std::vector<std::pair<int, int>> results;
    for(int size: {1024/32, 1024*4/32, 1024*16/32, 1024*32/32}){
        //issue is size is 32 so everything indexes into 32
        std::vector<std::pair<bool, std::uint64_t>> cache(size);
        std::pair<int, int> result = std::make_pair(0, 0);
        for (auto p : t.accesses) {
            //Calculate the cache line and the index
            std::uint32_t line = p.first - p.first%32;
            int index = (p.first%(size*32))/32;
            if(cache[index].first && cache[index].second == line)
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
    for(int assoc: {2, 4, 8, 16}){
        int size = ((1024*16)/32)/assoc;
        std::vector<std::vector<std::pair<bool, std::uint64_t>>> 
            cache(size, std::vector<std::pair<bool, std::uint64_t>>(assoc));
        std::vector<LRU> least_used(size);
        std::pair<int, int> result = std::make_pair(0, 0);

        for (auto p : t.accesses) {
            //Calculate the cache line and the index
            std::uint32_t line = p.first - p.first%32;
            int index = (p.first%(size*32))/32;

            auto& ways = cache[index];
            auto& lru = least_used[index];
            bool hit = false;
            //look for a hit
            for(int i = 0; i < assoc; ++i){
                if(ways[i].first && ways[i].second == line){
                    hit = true;
                    lru.access(i);
                    break;
                }
            }
            if(hit)
                result.first++;
            else if(!hit){
                //find an empty spot in the cache
                bool filled = false;
                for (int i = 0; i < assoc; ++i) {
                    if (ways[i].first == false) {
                        lru.access(i);
                        ways[i] = std::make_pair(true, line);
                        filled = true;
                        break;
                    }
                }
                //otherwise evict something
                if(!filled){
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


int main(int argc, char** argv) {
    if(argc != 3){
        std::cout << "ERROR WRONG ARGS\n";
        exit(0);
    }
    Trace t = read(argv[1]);
    std::string output = argv[2];

    direct_mapped(t).print(output, true);
    set_associative(t).print(output);
}