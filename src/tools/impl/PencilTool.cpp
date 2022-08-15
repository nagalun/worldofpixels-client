#include "PencilTool.hpp"

#include <cstdio>

#include <util/misc.hpp>
#include <InputManager.hpp>

#include <tools/ToolManager.hpp>
#include <tools/providers/ColorProvider.hpp>

#include <world/World.hpp>
#include <world/SelfCursor.hpp>

struct PencilTool::LocalContext {
	ImAction iSelectTool;
	ImAction iDrawPrimaryClr;
	ImAction iDrawSecondaryClr;

	World::Pos lastX;
	World::Pos lastY;

	LocalContext(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iDrawPrimaryClr(ia, "Draw Primary Color", T_ONPRESS | T_ONMOVE | T_ONHOLD),
	  iDrawSecondaryClr(ia, "Draw Secondary Color", T_ONPRESS | T_ONMOVE | T_ONHOLD),
	  lastX(0),
	  lastY(0) {

		iSelectTool.setDefaultKeybind("B");
		iDrawPrimaryClr.setDefaultKeybind(P_MPRIMARY);
		iDrawSecondaryClr.setDefaultKeybind(P_MSECONDARY);
	}

	void setLastPoint(World::Pos x, World::Pos y) {
		lastX = x;
		lastY = y;
	}

	World::Pos getLastX() const {
		return lastX;
	}

	World::Pos getLastY() const {
		return lastY;
	}
};

// local ctor
PencilTool::PencilTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  lctx(std::make_unique<LocalContext>(std::get<1>(params).mkAdapter(getNameSt()))) {

	World& w = tm.getWorld();
	// TODO: research whether SelfCursor should have its own events that may integrate with
	// InputManager somehow to keep keybind api but listen to something like T_ONPLAYERMOVE
	// T_ONHOLD is a dumb workaround for that issue, moving camera moves player without T_ONMOVE
	SelfCursor& sc = w.getCursor();
	ColorProvider& clr = tm.get<ColorProvider>();

	lctx->iSelectTool.setCb([this] (ImAction::Event& e, const InputInfo& ii) {
		tm.selectTool<PencilTool>();
	});

	const auto drawHandler = [&] (auto plotter) {
		return [&, plotter{std::move(plotter)}] (ImAction::Event& e, const InputInfo& ii) {
			switch (e.getActivationType()) {
			case T_ONPRESS:
				lctx->setLastPoint(sc.getX(), sc.getY());
				plotter(sc.getX(), sc.getY());
				break;

			case T_ONMOVE:
			case T_ONHOLD:
				if (sc.getX() != lctx->getLastX() || sc.getY() != lctx->getLastY()) {
					line(lctx->getLastX(), lctx->getLastY(), sc.getX(), sc.getY(), plotter);
					lctx->setLastPoint(sc.getX(), sc.getY());
				}
				break;

			default:
				break;
			}
		};
	};

	lctx->iDrawPrimaryClr.setCb(drawHandler([&sc, &clr] (auto x, auto y) {
		sc.paint(x, y, clr.getPrimaryColor());
	}));

	lctx->iDrawSecondaryClr.setCb(drawHandler([&sc, &clr] (auto x, auto y) {
		sc.paint(x, y, clr.getSecondaryColor());
	}));

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
	if (!lctx) {
		return;
	}

	lctx->iDrawPrimaryClr.setEnabled(selected);
	lctx->iDrawSecondaryClr.setEnabled(selected);
}

bool PencilTool::isEnabled() {
	return true;
}

std::uint8_t PencilTool::getNetId() const {
	return 0;
}

