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
inline void println(const std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

inline void println() {
    std::cout << '\n';
}

template<typename... Args>
inline void print(const std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...);
}
#endif

// Wrapper type for formatting std::vector<int> (avoids UB of specializing std::formatter)
struct FormattedVector {
    const std::vector<int>& v;
};

inline std::string format_as(const FormattedVector& fv) {
    std::string s = "{";
    for (size_t i = 0; i < fv.v.size(); ++i) {
        if (i > 0) s += ", ";
        s += std::to_string(fv.v[i]);
    }
    s += "}";
    return s;
}

namespace std {
template<>
struct formatter<FormattedVector> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const FormattedVector& fv, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", format_as(fv));
    }
};
}
