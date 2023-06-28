#include "Tool.hpp"

#include "tools/ToolManager.hpp"
#include <cstdint>

Tool::Tool(ToolManager& tm)
: tm(tm) { }

Tool::~Tool() { }

std::uint8_t Tool::getToolVisualState(const ToolStates&) const {
	return 0;
}

std::uint64_t Tool::getNetState(const ToolStates&) const {
	return 0;
}

bool Tool::setStateFromNet(ToolStates&, std::uint64_t) {
	return false;
}

bool Tool::operator ==(const Tool& t) const {
	return getName() == t.getName();
}
