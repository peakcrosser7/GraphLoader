#pragma once

#include <cstdlib>
#include <type_traits>

namespace graph_loader {

struct empty_t {};

template <typename T>
typename std::enable_if_t<std::is_integral_v<T>, T> 
general_weight_parse(const char* str) {
    if (!str) {
        return T(1);
    }
    return T(std::strtoull(str, nullptr, 10));
}

template <typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, T>
general_weight_parse(const char* str) {
    if (!str) {
        return T(1);
    }
    return T(std::strtod(str, nullptr));
}

// CANNOT use function-specialization for SFINAE
template <typename T>
typename std::enable_if_t<std::is_same_v<T, empty_t>, T> 
general_weight_parse(const char* str) {
    return {};
}

// use lambda to help decltype
constexpr auto dummy_func = []() {};

} // namespace graph_loader
