#pragma once
#include "ToolStates.hpp"

template<typename T>
constexpr typename T::State& ToolStates::get() {
	return std::get<typename T::State>(states);
}

template<typename T>
constexpr const typename T::State& ToolStates::get() const {
	return std::get<typename T::State>(states);
}
