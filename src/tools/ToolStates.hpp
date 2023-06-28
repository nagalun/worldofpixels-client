#pragma once

#include <cstdint>
#include "ToolManager.hpp"

// this class represents the state of the entire toolset, used by both the self and remote player.
class ToolStates {
	ToolManager::StateTuple states;
	std::uint8_t selectedTid;

public:
	ToolStates(std::uint8_t selectedTid);

	std::uint8_t getSelectedToolNetId() const;
	void setSelectedToolNetId(std::uint8_t st);

	// get a specific tool or provider's state from the tuple
	template<typename T>
	constexpr typename T::State& get();

	template<typename T>
	constexpr const typename T::State& get() const;
};

#include "ToolStates.tpp" // IWYU pragma: keep
