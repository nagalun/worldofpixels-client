#include "MoveTool.hpp"

#include <cstdio>
#include <cmath>

#include <InputManager.hpp>
#include <tools/ToolManager.hpp>
#include <world/World.hpp>
#include <Camera.hpp>

struct MoveTool::Keybinds {
	ImAction iSelectTool;
	ImAction iCamUp;
	ImAction iCamDown;
	ImAction iCamLeft;
	ImAction iCamRight;
	ImAction iCamZoomIn;
	ImAction iCamZoomOut;
	ImAction iCamZoomWh;
	ImAction iCamPanWh;
	ImAction iCamPanMo;
	ImAction iCamPanMoGl;
	ImAction iCamTouch;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iCamUp(ia, "Camera ↑", T_ONHOLD),
	  iCamDown(ia, "Camera ↓", T_ONHOLD),
	  iCamLeft(ia, "Camera ←", T_ONHOLD),
	  iCamRight(ia, "Camera →", T_ONHOLD),
	  iCamZoomIn(ia, "Zoom +"),
	  iCamZoomOut(ia, "Zoom -"),
	  iCamZoomWh(ia, "Zoom Wheel", T_ONWHEEL),
	  iCamPanWh(ia, "Pan Camera Wheel", T_ONWHEEL),
	  iCamPanMo(ia, "Pan Camera Mouse", T_ONPRESS | T_ONMOVE),
	  iCamPanMoGl(ia, "Pan Camera Mouse (Global)", T_ONPRESS | T_ONMOVE),
	  iCamTouch(ia, "Camera Touch Control", T_ONPRESS | T_ONMOVE) {

		iSelectTool.setDefaultKeybind("V");

		iCamUp.setDefaultKeybind("ARROWUP");
		iCamDown.setDefaultKeybind("ARROWDOWN");
		iCamLeft.setDefaultKeybind("ARROWLEFT");
		iCamRight.setDefaultKeybind("ARROWRIGHT");

		iCamZoomIn.setDefaultKeybind({M_CTRL, "+"});
		iCamZoomOut.setDefaultKeybind({M_CTRL, "-"});

		iCamPanMo.setDefaultKeybind(P_MPRIMARY);
		iCamPanMoGl.setDefaultKeybind(P_MMIDDLE);
		iCamTouch.setDefaultKeybind(P_MPRIMARY);
	}
};

// local ctor
MoveTool::MoveTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  kb(std::make_unique<Keybinds>(std::get<1>(params).mkAdapter(getNameSt()))) {

	Camera& c = tm.getWorld().getCamera();

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

	kb->iCamZoomIn.setCb([&] (auto&, const auto&) {
		float nz = c.getZoom() * 2.f;
		c.setZoom(nz >= 32.f ? 32.f : nz);
	});

	kb->iCamZoomOut.setCb([&] (auto&, const auto&) {
		float nz = c.getZoom() / 2.f;
		c.setZoom(nz);
	});

	kb->iCamZoomWh.setCb([&] (auto& ev, const auto& ii) {
		if (!(ii.getModifiers() & M_CTRL)) {
			ev.reject();
			return;
		}

		double d = ii.getWheelDy();
		if (d == 0.0) {
			return;
		}

		float nz = std::min(32.f, std::max(1.f, c.getZoom() - (d > 0.0 ? 1.f : -1.f)));
		c.setZoom(nz);
	});

	kb->iCamPanWh.setCb([&] (auto& ev, const auto& ii) {
		if (ii.getModifiers() & M_CTRL) {
			ev.reject();
			return;
		}

		float z = c.getZoom();
		c.translate(ii.getWheelDx() / z, ii.getWheelDy() / z);
	});

	const auto moPan = [&] (auto& ev, const auto& ii) {
		c.translate(
			-ii.getDx() / c.getZoom(),
			-ii.getDy() / c.getZoom()
		);
	};

	kb->iCamPanMo.setCb(moPan);
	kb->iCamPanMoGl.setCb(moPan);

	kb->iCamTouch.setCb([
		&,
		lastDist{0.0}
	] (auto& ev, const auto& ii) mutable {
		const auto& p = ii.getActivePointers();
		if (p.size() != 2) {
			ev.reject();
			return;
		}

		double dist = std::sqrt(std::pow(p[1]->getX() - p[0]->getX(), 2) + std::pow(p[1]->getY() - p[0]->getY(), 2));
		if (ev.getActivationType() == T_ONPRESS) {
			lastDist = dist;
			return;
		}

		int lastMidX = (p[1]->getLastX() + p[0]->getLastX()) / 2;
		int lastMidY = (p[1]->getLastY() + p[0]->getLastY()) / 2;
		int midX = (p[1]->getX() + p[0]->getX()) / 2;
		int midY = (p[1]->getY() + p[0]->getY()) / 2;

		int midDx = midX - lastMidX;
		int midDy = midY - lastMidY;

		c.setZoom(std::max(1.f, c.getZoom() * (float)(dist / lastDist)));
		c.translate(
			-midDx / c.getZoom(),
			-midDy / c.getZoom()
		);

		lastDist = dist;
	});

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

	kb->iCamPanMo.setEnabled(selected);
	kb->iCamTouch.setEnabled(selected);
}

bool MoveTool::isEnabled() {
	return true;
}

std::uint8_t MoveTool::getNetId() const {
	return 1;
}

