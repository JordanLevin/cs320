#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct Result {
    // Correct predictions and total branches
    std::vector<std::pair<int, int>> res;
    void print(std::string f) {
        std::ofstream file(f, std::ios::app);
        for (int i = 0; i < res.size(); i++) {
            file << res[i].first << ',' << res[i].second << ';';
            if (i != res.size() - 1) file << ' ';
        }
        file << '\n';
        file.close();
    }
};

struct Trace {
    std::vector<std::pair<std::uint32_t, bool>> branches;
    void print() {
        for (const auto& p : branches) {
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

        std::string take;
        std::uint32_t pc;
        bool b;
        s >> std::hex >> pc;
        s >> take;
        b = (take == "T");
        ret.branches.push_back(std::make_pair(pc, b));
    }
    file.close();
    return ret;
}

Result always_taken(const Trace& t) {
    Result ret;
    std::pair<int, int> result = std::make_pair(0, 0);
    for (std::pair<std::uint32_t, bool> p : t.branches) {
        if (p.second) result.first++;
        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result always_not_taken(const Trace& t) {
    Result ret;
    std::pair<int, int> result = std::make_pair(0, 0);
    for (std::pair<std::uint32_t, bool> p : t.branches) {
        if (!p.second) result.first++;
        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result bimodal_single(const Trace& t) {
    Result ret;
    for (int size : {16, 32, 128, 256, 512, 1024, 2048}) {
        // Setup table to be false and initialize result
        std::pair<int, int> result = std::make_pair(0, 0);
        std::vector<bool> table(size);
        for (int i = 0; i < table.size(); i++) table[i] = false;

        // Simulate predictions
        for (const auto& p : t.branches) {
            if (table[p.first % size] == p.second)
                result.first++;
            else
                table[p.first % size] = p.second;
            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}

Result bimodal_double(const Trace& t) {
    Result ret;
    for (int size : {16, 32, 128, 256, 512, 1024, 2048}) {
        // Setup table to be false and initialize result
        std::pair<int, int> result = std::make_pair(0, 0);
        std::vector<int> table(size, 1);

        // Simulate predictions
        for (const auto& p : t.branches) {
            if ((table[p.first % size] > 1) == p.second) result.first++;

            if (p.second == false)
                table[p.first % size] = std::max(0, table[p.first % size] - 1);
            else if (p.second == true)
                table[p.first % size] = std::min(3, table[p.first % size] + 1);

            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}

Result gshare(const Trace& t) {
    Result ret;
    int size = 2048;
    for (int hist = 3; hist < 12; hist++) {
        // Setup table to be false and initialize result
        std::pair<int, int> result = std::make_pair(0, 0);
        std::vector<int> table(size, 1);
        uint16_t ghist = 0;
        uint16_t mask = 0xffff >> (16 - hist);

        // Simulate predictions
        for (const auto& p : t.branches) {
            // may need to fix xor logic
            uint16_t index = (ghist) ^ (p.first % size);

            if ((table[index] > 1) == p.second) result.first++;

            if (p.second == false)
                table[index] = std::max(0, table[index] - 1);
            else if (p.second == true)
                table[index] = std::min(3, table[index] + 1);

            result.second++;
            ghist <<= 1;
            ghist |= p.second;
            ghist &= mask;
        }
        ret.res.push_back(result);
    }
    return ret;
}

Result tournament(const Trace& t) {
    Result ret;
    int size = 2048;
    int hist = 11;
    // Setup table to be false and initialize result
    std::pair<int, int> result = std::make_pair(0, 0);
    std::vector<int> gtable(size, 1);
    std::vector<int> btable(size, 1);
    std::vector<int> select(size,
                            2);  // 0 - 3 strongly gshare to strongly bimodal
    uint16_t ghist = 0;
    uint16_t mask = 0xffff >> (16 - hist);

    // Simulate predictions
    for (const auto& p : t.branches) {
        bool gres = false;
        bool bres = false;

        // GSHARE
        uint16_t index = (ghist) ^ (p.first % size);

        if ((gtable[index] > 1) == p.second) gres = true;
        if (p.second == false)
            gtable[index] = std::max(0, gtable[index] - 1);
        else if (p.second == true)
            gtable[index] = std::min(3, gtable[index] + 1);

        ghist <<= 1;
        ghist |= p.second;
        ghist &= mask;
        // BIMODAL
        int bindex = p.first%size;
        if ((btable[bindex] > 1) == p.second) bres = true;

        if (p.second == false)
            btable[bindex] = std::max(0, btable[bindex] - 1);
        else if (p.second == true)
            btable[bindex] = std::min(3, btable[bindex] + 1);

        // COMBINE THEM WITH TOURNAMENT
        if (select[bindex] > 1) {
            if (bres) result.first++;
        } else {
            if (gres) result.first++;
        }

        if ((bres && !gres))
            select[bindex] = std::min(3, select[bindex] + 1);
        else if ((!bres && gres))
            select[bindex] = std::max(0, select[bindex] - 1);

        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

int main(int argc, char** argv) {
    if(argc != 3){
        std::cout << "ERROR WRONG ARGS\n";
        exit(0);
    }
    Trace t = read(argv[1]);
    std::string output = argv[2];

    always_taken(t).print(output);
    always_not_taken(t).print(output);
    bimodal_single(t).print(output);
    bimodal_double(t).print(output);
    gshare(t).print(output);
    tournament(t).print(output);
    //perceptron(t).print(output);
}
