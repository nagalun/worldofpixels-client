#include "ToolStates.hpp"

ToolStates::ToolStates(std::uint8_t selectedTid)
: selectedTid(selectedTid) { }

std::uint8_t ToolStates::getSelectedToolNetId() const {
	return selectedTid;
}

void ToolStates::setSelectedToolNetId(std::uint8_t st) {
	selectedTid = st;
}
