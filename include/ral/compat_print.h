#pragma once
#include <iostream>
#include <format>
#include <vector>
#include <string>

#if __has_include(<print>)
#include <print>
using std::print;
using std::println;
#else
template<typename... Args>
void println(const std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

inline void println() {
    std::cout << '\n';
}

template<typename... Args>
void print(const std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...);
}
#endif

namespace std {
template<>
struct formatter<vector<int>> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const vector<int>& v, FormatContext& ctx) const {
        string s = "{";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) s += ", ";
            s += to_string(v[i]);
        }
        s += "}";
        return std::format_to(ctx.out(), "{}", s);
    }
};
}
