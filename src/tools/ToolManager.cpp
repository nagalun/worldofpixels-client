#include "ToolManager.hpp"

#include <utility>

#include "PacketDefinitions.hpp"
#include "ToolStates.hpp"
#include "tools/Tool.hpp"
#include "util/misc.hpp"
#include "InputManager.hpp"

ToolManager::ToolManager(World& w, ToolStates& localState, InputAdapter& ia)
: w(w),
  localState(localState),
  providers(cartesian_make_tuple<ProvidersTuple>(std::forward_as_tuple(*this, ia.mkAdapter("Tools")))),
  tools(cartesian_make_tuple<ToolsTuple>(std::forward_as_tuple(*this, ia.mkAdapter("Tools")))) { }

World& ToolManager::getWorld() {
	return w;
}

ToolStates& ToolManager::getLocalState() {
	return localState;
}

Tool* ToolManager::getByNetId(std::uint8_t id) {
	Tool* r = nullptr;
	// TODO: optimize this
	forEachTool([id, &r] (int i, Tool& t) {
		if (id == t.getNetId()) {
			r = &t;
		}
	});

	return r;
}

Tool* ToolManager::getSelectedTool() {
	return getSelectedTool(localState);
}

Tool* ToolManager::getSelectedTool(const ToolStates& ts) {
	return getByNetId(ts.getSelectedToolNetId());
}

void ToolManager::selectTool(Tool* newTool) {
	ToolStates& ts = getLocalState();
	Tool* oldTool = getSelectedTool();

	if (oldTool == newTool) {
		return;
	}

	std::uint8_t newTid = net::ToolId::TID_UNKNOWN;

	if (oldTool != nullptr) {
		oldTool->onSelectionChanged(false);
	}

	if (newTool != nullptr) {
		newTool->onSelectionChanged(true);
		newTid = newTool->getNetId();
	}

	ts.setSelectedToolNetId(newTid);

	onLocalStateChanged(ts, newTool);
}

std::uint64_t ToolManager::getState(const ToolStates& ts) {
	Tool* t = getSelectedTool(ts);
	if (!t) {
		return 0;
	}

	return t->getNetState(ts);
}

bool ToolManager::updateState(ToolStates& ts, std::uint8_t newTid, std::uint64_t newState) {
	bool updated = ts.getSelectedToolNetId() != newTid;
	ts.setSelectedToolNetId(newTid);
	Tool* t = getSelectedTool(ts);

	if (t) {
		updated |= t->setStateFromNet(ts, newState);
	}

	return updated;
}
