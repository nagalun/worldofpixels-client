#include <functional>

template <typename T, typename... Args>
void hash_combine(std::size_t& seed, const T& v, const Args&... args) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, args), ...);
}
