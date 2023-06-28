#include "PipetteTool.hpp"

#include <cstdio>
#include <cmath>

#include "InputManager.hpp"
#include "world/World.hpp"
#include "tools/ToolManager.hpp"
#include "tools/providers/ColorProvider.hpp"

PipetteTool::State::State()
: clicking(false) { }

bool PipetteTool::State::setClicking(bool st) {
	bool changed = st != clicking;
	clicking = st;
	return changed;
}

bool PipetteTool::State::isClicking() const {
	return clicking;
}

struct PipetteTool::Keybinds {
	ImAction iSelectTool;
	ImAction iPickPrimaryColor;
	ImAction iPickSecondaryColor;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iPickPrimaryColor(ia, "Pick primary color", T_ONPRESS | T_ONMOVE | T_ONRELEASE),
	  iPickSecondaryColor(ia, "Pick secondary color", T_ONPRESS | T_ONMOVE | T_ONRELEASE) {

		iSelectTool.setDefaultKeybind("I");

		iPickPrimaryColor.setDefaultKeybind(P_MPRIMARY);
		iPickSecondaryColor.setDefaultKeybind(P_MSECONDARY);
	}
};

// local ctor
PipetteTool::PipetteTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  kb(std::make_unique<Keybinds>(std::get<1>(params).mkAdapter(getNameSt()))) {

	World& w = tm.getWorld();
	SelfCursor& sc = w.getCursor();
	ColorProvider::State& clr = tm.getLocalState().get<ColorProvider>();
	PipetteTool::State& st = tm.getLocalState().get<PipetteTool>();

	kb->iSelectTool.setCb([this] (auto&, const auto&) {
		tm.selectTool<PipetteTool>();
	});

	kb->iPickPrimaryColor.setCb([&] (auto&, const InputInfo& ii) {
		bool upd = false;
		upd |= clr.setPrimaryColor(w.getPixel(sc.getX(), sc.getY()));
		upd |= st.setClicking(ii.getNumActivePointers() > 0);

		if (upd) {
			tm.emitLocalStateChanged<PipetteTool>();
		}
	});

	kb->iPickSecondaryColor.setCb([&] (auto&, const InputInfo& ii) {
		bool upd = false;
		upd |= clr.setSecondaryColor(w.getPixel(sc.getX(), sc.getY()));
		upd |= st.setClicking(ii.getNumActivePointers() > 0);

		if (upd) {
			tm.emitLocalStateChanged<PipetteTool>();
		}
	});

	onSelectionChanged(false);
}

PipetteTool::~PipetteTool() { }

const char * PipetteTool::getNameSt() {
	return "Pipette";
}

std::string_view PipetteTool::getName() const {
	return getNameSt();
}

std::string_view PipetteTool::getToolVisualName(const ToolStates&) const {
	return "pipette";
}

void PipetteTool::onSelectionChanged(bool selected) {
	kb->iPickPrimaryColor.setEnabled(selected);
	kb->iPickSecondaryColor.setEnabled(selected);
}

bool PipetteTool::isEnabled() {
	return true;
}

std::uint8_t PipetteTool::getNetId() const {
	return net::ToolId::TID_PIPETTE;
}

std::uint64_t PipetteTool::getNetState(const ToolStates& ts) const {
	const auto& st = ts.get<PipetteTool>();
	return st.isClicking();
}

bool PipetteTool::setStateFromNet(ToolStates& ts, std::uint64_t netState) {
	auto& st = ts.get<PipetteTool>();
	return st.setClicking(netState & 1);
}
