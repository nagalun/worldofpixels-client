#pragma once

#include <cstdlib>

template <typename T, typename... Args>
void hash_combine(std::size_t& seed, const T& v, const Args&... args);

#include "util/hash_combine.tpp"
