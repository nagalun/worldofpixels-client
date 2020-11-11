#pragma once

#include <type_traits>
#include <tuple>
#include <array>
#include <optional>

// https://stackoverflow.com/a/30848101
// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf.
template <typename...>
using void_t = void;

// Primary template handles all types not supporting the operation.
template <typename, template <typename> class, typename = void_t<>>
struct detect : std::false_type {};

// Specialization recognizes/validates only types supporting the archetype.
template <typename T, template <typename> class Op>
struct detect<T, Op, void_t<Op<T>>> : std::true_type {};


template<typename... Types>
struct are_all_arithmetic;

template<>
struct are_all_arithmetic<> : std::true_type {};

template<typename T, typename... Types>
struct are_all_arithmetic<T, Types...> : std::integral_constant<bool,
	std::is_arithmetic<T>::value &&
	are_all_arithmetic<Types...>::value> {};

// Helper to determine whether there's a const_iterator for T.
template<typename T>
struct has_const_iterator {
private:
    template<typename C> static char test(typename C::const_iterator*);
    template<typename C> static int  test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

template<typename>
struct is_std_array : std::false_type {};

template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<typename>
struct is_tuple : std::false_type {};

template<typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

template<typename>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<typename... Args>
constexpr decltype(auto) add(Args&&... args) {
	return (args + ... + 0);
}

template<typename>
struct is_tuple_arithmetic : std::false_type {};

template<typename... Ts>
struct is_tuple_arithmetic<std::tuple<Ts...>> : are_all_arithmetic<Ts...> {
	// member could be removed if value is false?
	static constexpr std::size_t size = add(sizeof(Ts)...);
};
