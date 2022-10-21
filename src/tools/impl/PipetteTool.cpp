#include "PipetteTool.hpp"

#include <cstdio>
#include <cmath>

#include "InputManager.hpp"
#include "world/World.hpp"
#include "tools/ToolManager.hpp"
#include "tools/providers/ColorProvider.hpp"

struct PipetteTool::Keybinds {
	ImAction iSelectTool;
	ImAction iPickPrimaryColor;
	ImAction iPickSecondaryColor;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iPickPrimaryColor(ia, "Pick primary color", T_ONPRESS | T_ONMOVE),
	  iPickSecondaryColor(ia, "Pick secondary color", T_ONPRESS | T_ONMOVE) {

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
	ColorProvider& clr = tm.get<ColorProvider>();

	kb->iSelectTool.setCb([this] (auto&, const auto&) {
		tm.selectTool<PipetteTool>();
	});

	kb->iPickPrimaryColor.setCb([&] (auto&, const auto&) {
		clr.setPrimaryColor(w.getPixel(sc.getX(), sc.getY()));
	});

	kb->iPickSecondaryColor.setCb([&] (auto&, const auto&) {
		clr.setSecondaryColor(w.getPixel(sc.getX(), sc.getY()));
	});

	onSelectionChanged(false);
}

// remote ctor
PipetteTool::PipetteTool(ToolManager& tm)
: Tool(tm) { }

PipetteTool::~PipetteTool() { }

const char * PipetteTool::getNameSt() {
	return "Pipette";
}

std::string_view PipetteTool::getName() const {
	return getNameSt();
}

void PipetteTool::onSelectionChanged(bool selected) {
	if (!kb) {
		return;
	}

	kb->iPickPrimaryColor.setEnabled(selected);
	kb->iPickSecondaryColor.setEnabled(selected);
}

bool PipetteTool::isEnabled() {
	return true;
}

std::uint8_t PipetteTool::getNetId() const {
	return 1;
}
