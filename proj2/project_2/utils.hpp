#include <vector>
#include <utility>
#include <fstream>
#include <iostream>

#ifndef UTILS_H
#define UTILS_H

struct Result {
    // Cache hits and total accesses
    std::vector<std::pair<int, int>> res;
    void print(std::string f, bool overwrite = false) {
        std::ofstream file;
        if(overwrite)
            file = std::ofstream(f);
        else
            file = std::ofstream(f, std::ios::app);
        for (int i = 0; i < res.size(); i++) {
            file << res[i].first << ',' << res[i].second << ';';
            if (i != res.size() - 1) file << ' ';
        }
        file << '\n';
        file.close();
    }
};

struct Trace {
    std::vector<std::pair<std::uint32_t, bool>> accesses;
    void print() {
        for (const auto& p : accesses) {
            std::cout << std::hex << p.first << " " << p.second << std::endl;
        }
    }
};

Trace read(std::string f) {
    std::ifstream file(f);

    Trace ret;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream s(line);

        std::string storeload;
        std::uint32_t pc;
        bool b;
        s >> storeload;
        s >> std::hex >> pc;
        b = (storeload == "S");
        ret.accesses.push_back(std::make_pair(pc, b));
    }
    file.close();
    return ret;
}

#endif
