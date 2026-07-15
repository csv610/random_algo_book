#pragma once

#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

namespace chapter1 {

class RandomEngine {
public:
    static RandomEngine& instance() {
        static RandomEngine eng;
        return eng;
    }
    
    std::mt19937& engine() { return engine_; }
    
    int rand_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine_);
    }
    
    double rand_double(double min = 0.0, double max = 1.0) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(engine_);
    }
    
    bool coin_flip(double p = 0.5) {
        return rand_double() < p;
    }
    
    template<typename T>
    void shuffle(std::vector<T>& vec) {
        std::shuffle(vec.begin(), vec.end(), engine_);
    }
    
    // Geometric distribution - number of trials until first success
    int geometric(double p) {
        std::geometric_distribution<int> dist(p);
        return dist(engine_);
    }

private:
    RandomEngine() : engine_(std::chrono::steady_clock::now().time_since_epoch().count()) {}
    std::mt19937 engine_;
};

inline RandomEngine& rng() {
    return RandomEngine::instance();
}

} // namespace chapter1