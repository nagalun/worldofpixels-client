#pragma once

#include <string_view>
#include <type_traits>
#include <optional>

template<typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_floating_point_v<T>, int> = 0>
std::optional<T> fromString(std::string_view, Args... fmtArgs);

template<typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, bool> && std::is_floating_point_v<T>, int> = 0>
std::optional<T> fromString(std::string_view, Args... fmtArgs);

template<typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
std::optional<T> fromString(std::string_view);

template<typename T, typename... Args>
std::string_view toString(T value, Args... fmtArgs);
