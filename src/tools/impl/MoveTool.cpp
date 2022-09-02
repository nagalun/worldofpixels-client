#include "MoveTool.hpp"

#include <cstdio>
#include <cmath>

#include <InputManager.hpp>
#include <tools/ToolManager.hpp>
#include <world/World.hpp>
#include <world/SelfCursor.hpp>
#include <Camera.hpp>

struct MoveTool::Keybinds {
	ImAction iSelectTool;
	ImAction iCamUp;
	ImAction iCamDown;
	ImAction iCamLeft;
	ImAction iCamRight;
	ImAction iCamPanWh;
	ImAction iCamPan;
	ImAction iCamPanGl;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iCamUp(ia, "Camera ↑", T_ONHOLD),
	  iCamDown(ia, "Camera ↓", T_ONHOLD),
	  iCamLeft(ia, "Camera ←", T_ONHOLD),
	  iCamRight(ia, "Camera →", T_ONHOLD),
	  iCamPanWh(ia, "Pan Camera Wheel", T_ONWHEEL),
	  iCamPan(ia, "Pan Camera", T_ONPRESS | T_ONMOVE),
	  iCamPanGl(ia, "Pan Camera (Global)", T_ONPRESS | T_ONMOVE) {

		iSelectTool.setDefaultKeybind("V");

		iCamUp.setDefaultKeybind("ARROWUP");
		iCamDown.setDefaultKeybind("ARROWDOWN");
		iCamLeft.setDefaultKeybind("ARROWLEFT");
		iCamRight.setDefaultKeybind("ARROWRIGHT");

		iCamPan.setDefaultKeybind(P_MPRIMARY);
		iCamPanGl.setDefaultKeybind(P_MMIDDLE);
	}
};

// local ctor
MoveTool::MoveTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  kb(std::make_unique<Keybinds>(std::get<1>(params).mkAdapter(getNameSt()))) {

	Camera& c = tm.getWorld().getCamera();
	SelfCursor& sc = tm.getWorld().getCursor();

	kb->iSelectTool.setCb([this] (auto&, const auto&) {
		tm.selectTool<MoveTool>();
	});

	kb->iCamUp.setCb([&] (auto&, const auto&) {
		c.translate(0.f, -3.f / (c.getZoom() / 16.f));
	});

	kb->iCamDown.setCb([&] (auto&, const auto&) {
		c.translate(0.f, 3.f / (c.getZoom() / 16.f));
	});

	kb->iCamLeft.setCb([&] (auto&, const auto&) {
		c.translate(-3.f / (c.getZoom() / 16.f), 0.f);
	});

	kb->iCamRight.setCb([&] (auto&, const auto&) {
		c.translate(3.f / (c.getZoom() / 16.f), 0.f);
	});

	kb->iCamPanWh.setCb([&] (auto& ev, const auto& ii) {
		if (ii.getModifiers() & M_CTRL) {
			ev.reject();
			return;
		}

		float z = c.getZoom();
		c.translate(ii.getWheelDx() / z, ii.getWheelDy() / z);
	});

	const auto panCb = [
		&,
		lastDist{0.f}
	] (auto& ev, const auto& ii) mutable {
//		using EType = InputInfo::Pointer::EType;
//		if (ii.getType() != EType::TOUCH) {
//			ev.reject();
//			return;
//		}

		const auto& po = ii.getPointers();

		if (ii.getNumActivePointers() > 1) {
			float dist = std::sqrt(std::pow(po[1]->getX() - po[0]->getX(), 2.f) + std::pow(po[1]->getY() - po[0]->getY(), 2.f));
			if (ev.getActivationType() == T_ONPRESS) {
				lastDist = dist;
				return;
			}

			c.setZoom(std::max(1.f, c.getZoom() * (dist / lastDist)), sc.getFinalX(), sc.getFinalY());
			lastDist = dist;
		}

		c.translate(
			-ii.getMidDx() / c.getZoom(),
			-ii.getMidDy() / c.getZoom()
		);
	};

	kb->iCamPan.setCb(panCb);
	kb->iCamPanGl.setCb(panCb);

	onSelectionChanged(false);
}

// remote ctor
MoveTool::MoveTool(ToolManager& tm)
: Tool(tm) { }

MoveTool::~MoveTool() { }

const char * MoveTool::getNameSt() {
	return "Move";
}

std::string_view MoveTool::getName() const {
	return getNameSt();
}

void MoveTool::onSelectionChanged(bool selected) {
	if (!kb) {
		return;
	}

	kb->iCamPan.setEnabled(selected);
}

bool MoveTool::isEnabled() {
	return true;
}

std::uint8_t MoveTool::getNetId() const {
	return 1;
}

