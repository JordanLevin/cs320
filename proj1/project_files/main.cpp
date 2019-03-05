#include <iostream>
#include <utility>
#include <cstdint>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

struct Result{
    //Correct predictions and total branches
    std::vector<std::pair<int, int>> res;
    void print(std::string f){
        std::ofstream file(f, std::ios::app);
        for(auto p: res){
            file << p.first << ',' << p.second << "; ";
        }
        file << '\n';
        file.close();
    }
};

struct Trace {
    std::vector<std::pair<std::uint32_t, bool>> branches;
    void print(){
        for(const auto& p: branches){
            std::cout << std::hex << p.first << " " << p.second << std::endl;
        }
    }
};

Trace read(std::string f){
    std::ifstream file(f);

    Trace ret;
    std::string line;
    while(std::getline(file, line)){
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

Result always_taken(const Trace& t){
    Result ret;
    std::pair<int, int> result = std::make_pair(0,0);
    for(std::pair<std::uint32_t, bool> p: t.branches){
        if(p.second)
            result.first++;
        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result always_not_taken(const Trace& t){
    Result ret;
    std::pair<int, int> result = std::make_pair(0,0);
    for(std::pair<std::uint32_t, bool> p: t.branches){
        if(!p.second)
            result.first++;
        result.second++;
    }
    ret.res.push_back(result);
    return ret;
}

Result bimodal_single(const Trace& t){
    Result ret;
    for(int size: {16,32,128,256,1024,2048}){
        //Setup table to be false and initialize result
        std::pair<int, int> result = std::make_pair(0,0);
        std::vector<bool> table(size);
        for(int i = 0; i < table.size(); i++)
            table[i] = false;

        //Simulate predictions
        for(const auto& p: t.branches){
            if(table[p.first%size] == p.second)
                result.first++;
            else
                table[p.first%size] = p.second;
            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}

//TODO
Result bimodal_double(const Trace& t){
    Result ret;
    for(int size: {16,32,128,256,1024,2048}){
        //Setup table to be false and initialize result
        std::pair<int, int> result = std::make_pair(0,0);
        std::vector<std::pair<bool, bool>> table(size);
        for(int i = 0; i < table.size(); i++)
            table[i] = std::make_pair(false, false);

        //Simulate predictions
        for(const auto& p: t.branches){
            if(table[p.first%size] == p.second)
                result.first++;
            else
                table[p.first%size] = p.second;
            result.second++;
        }
        ret.res.push_back(result);
    }
    return ret;
}



int main(int argc, char** argv){
    Trace t = read(argv[1]);
    std::string output = argv[2];

    //Hack to delete output file
    //std::fstream temp(output, std::ios::trunc);
    //temp << "BLEEEE";
    //temp.close();
    
    Result r1 = always_taken(t);
    r1.print(output);
    Result r2 = always_not_taken(t);
    r2.print(output);
    Result r3 = bimodal_single(t);
    r3.print(output);
}
