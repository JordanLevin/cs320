#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "utils.hpp"

#define HIST_SIZE 16

struct perceptron {
    std::vector<int> weights = std::vector<int>(HIST_SIZE, 0);
};

int d(bool b){
    if(b) return 1;
    return -1;
}

Result nn(const Trace& t) {
    Result ret;
    int size = 2048;
    std::vector<bool> hist(HIST_SIZE, false);
    // Setup table to be false and initialize result
    std::pair<int, int> result = std::make_pair(0, 0);
    std::vector<perceptron> table(size);

    // Simulate predictions
    for (const auto& branch : t.branches) {
        perceptron& p = table[branch.first%size];
        //Calculate prediction
        int predict = 0;
        for(int i = 0; i < p.weights.size(); i++){
            predict += p.weights[i] * d(hist[i]);
        }
        if(predict >= 0 && branch.second)
            result.first++;
        else if(predict < 0 && !branch.second)
            result.first++;
        else{
            //Update weights
            for(int i = 0; i < p.weights.size(); i++){
                if(branch.second == hist[i])
                    p.weights[i] += 1;
                else
                    p.weights[i] -= 1;
            }
        }

        //Update history
        hist.push_back(branch.second);
        hist.erase(hist.begin());

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

    nn(t).print(output);
}
