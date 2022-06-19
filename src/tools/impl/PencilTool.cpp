#include "PencilTool.hpp"

#include <cstdio>

#include <InputManager.hpp>
#include <tools/ToolManager.hpp>
#include <world/World.hpp>
#include <world/SelfCursor.hpp>

struct PencilTool::Keybinds {
	ImAction iSelectTool;
	ImAction iDrawPrimaryClr;
	ImAction iDrawSecondaryClr;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iDrawPrimaryClr(ia, "Draw Primary Color", T_ONPRESS | T_ONMOVE),
	  iDrawSecondaryClr(ia, "Draw Secondary Color", T_ONPRESS | T_ONMOVE) {

		iSelectTool.setDefaultKeybind("B");
		iDrawPrimaryClr.setDefaultKeybind(P_MPRIMARY);
		iDrawSecondaryClr.setDefaultKeybind(P_MSECONDARY);
	}
};

// local ctor
PencilTool::PencilTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  kb(std::make_unique<Keybinds>(std::get<1>(params).mkAdapter(getNameSt()))) {

	World& w = tm.getWorld();
	SelfCursor& sc = w.getCursor();

	kb->iSelectTool.setCb([this] (ImAction::Event& e, const InputInfo& ii) {
		tm.selectTool<PencilTool>();
	});

	kb->iDrawPrimaryClr.setCb([&] (ImAction::Event& e, const InputInfo& ii) {
		w.setPixel(sc.getX(), sc.getY(), {{255, 0, 0, 255}});
	});

	kb->iDrawSecondaryClr.setCb([&] (ImAction::Event& e, const InputInfo& ii) {
		std::printf("Draw secondary color\n");
	});

	onSelectionChanged(false);
}

// remote ctor
PencilTool::PencilTool(ToolManager& tm)
: Tool(tm) { }

PencilTool::~PencilTool() { }

const char * PencilTool::getNameSt() {
	return "Pencil";
}

std::string_view PencilTool::getName() const {
	return getNameSt();
}

void PencilTool::onSelectionChanged(bool selected) {
	if (!kb) {
		return;
	}

	kb->iDrawPrimaryClr.setEnabled(selected);
	kb->iDrawSecondaryClr.setEnabled(selected);
}

bool PencilTool::isEnabled() {
	return true;
}

std::uint8_t PencilTool::getNetId() const {
	return 0;
}

