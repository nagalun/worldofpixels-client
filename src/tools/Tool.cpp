#include "Tool.hpp"

#include <tools/ToolManager.hpp>

Tool::Tool(ToolManager& tm)
: tm(tm) { }

Tool::~Tool() { }

const std::vector<std::uint8_t>& Tool::getNetState() const {
	static const std::vector<std::uint8_t> empty;
	return empty;
}

void Tool::setStateFromNet(std::vector<std::uint8_t> allocator) {
}

bool Tool::operator ==(const Tool& t) const {
	return getName() == t.getName();
}
