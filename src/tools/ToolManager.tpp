#pragma once
#include "ToolManager.hpp"
#include <type_traits>

template<typename Fn>
constexpr void ToolManager::forEachTool(Fn cb) {
	std::apply([cb{std::move(cb)}] (auto&... x) {
		int i = 0;
		(cb(i++, x), ...);
	}, tools);
}

template<typename T>
constexpr T& ToolManager::get() {
	if constexpr (std::is_base_of_v<Tool, T>) {
		return std::get<T>(tools);
	} else {
		return std::get<T>(providers);
	}
}

template<typename T>
void ToolManager::selectTool() {
	selectTool(&std::get<T>(tools));
}
