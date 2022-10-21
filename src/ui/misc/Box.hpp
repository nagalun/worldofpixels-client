#pragma once

#include <cstdint>
#include <tuple>

#include "util/emsc/ui/Object.hpp"

template<typename... Content>
class Box : public eui::Object {
	std::tuple<Content...> content;

public:
	template<typename... CArgs>
	Box(CArgs&&...);

	template<typename T>
	T& get();

	template<typename T>
	const T& get() const;

	template<std::size_t I>
	const auto& get() const;
};

#include "Box.tpp" // IWYU pragma: keep
