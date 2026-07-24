#pragma once

#include <random>
#include <vector>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <span>
#include "compat_print.h"

namespace randalgo {

class RandomEngine {
public:
    static RandomEngine& instance() {
        static RandomEngine eng;
        return eng;
    }

    std::mt19937& engine() { return engine_; }

    int rand_int(int min, int max) {
        return std::uniform_int_distribution(min, max)(engine_);
    }

    double rand_double(double min = 0.0, double max = 1.0) {
        return std::uniform_real_distribution(min, max)(engine_);
    }

    bool coin_flip(double p = 0.5) {
        return rand_double() < p;
    }

    template<typename T>
    void shuffle(std::vector<T>& vec) {
        std::ranges::shuffle(vec, engine_);
    }

    template<std::floating_point T>
    std::vector<T> sample(std::span<const T> items, double p) {
        std::vector<T> result;
        for (const auto& item : items) {
            if (coin_flip(p)) result.push_back(item);
        }
        return result;
    }

private:
    RandomEngine()
        : engine_(static_cast<unsigned>(
              std::chrono::steady_clock::now().time_since_epoch().count())) {}
    std::mt19937 engine_;
};

inline RandomEngine& rng() {
    return RandomEngine::instance();
}

} // namespace randalgo
