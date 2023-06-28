#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "util/NonCopyable.hpp"

class ToolManager;
class ToolStates;

class Tool : NonCopyable {
protected:
	ToolManager& tm;

public:
	Tool(ToolManager&);

	virtual ~Tool();

	virtual std::string_view getName() const = 0;
	// the visual name and state is the horizontal and vertical index of the toolset atlas to show, respectively
	virtual std::string_view getToolVisualName(const ToolStates&) const = 0;
	virtual std::uint8_t getToolVisualState(const ToolStates&) const;
	virtual bool isEnabled() = 0;

	// note: an empty network state (0) must be valid, server may not send state in some circumstances
	virtual std::uint8_t getNetId() const = 0;
	virtual std::uint64_t getNetState(const ToolStates&) const;
	// a return value of true means the state changed meaningfully (i.e. appearance of the tool changed and needs rerender)
	virtual bool setStateFromNet(ToolStates&, std::uint64_t);

	virtual void onSelectionChanged(bool selected) = 0;

	bool operator==(const Tool&) const;
};
