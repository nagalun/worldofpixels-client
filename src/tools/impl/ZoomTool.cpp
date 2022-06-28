#include "ZoomTool.hpp"

#include <cstdio>

#include <InputManager.hpp>
#include <tools/ToolManager.hpp>
#include <world/World.hpp>
#include <world/SelfCursor.hpp>

struct ZoomTool::Keybinds {
	ImAction iSelectTool;
	ImAction iCamZoomIn;
	ImAction iCamZoomOut;
	ImAction iCamZoomInGl;
	ImAction iCamZoomOutGl;
	ImAction iCamZoomWhGl;

	Keybinds(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iCamZoomIn(ia, "Zoom +"),
	  iCamZoomOut(ia, "Zoom -"),
	  iCamZoomInGl(ia, "Zoom + (Global)"),
	  iCamZoomOutGl(ia, "Zoom - (Global)"),
	  iCamZoomWhGl(ia, "Zoom Wheel (Global)", T_ONWHEEL) {

		iSelectTool.setDefaultKeybind("Z");
		iCamZoomIn.setDefaultKeybind(P_MPRIMARY);
		iCamZoomOut.setDefaultKeybind(P_MSECONDARY);

		iCamZoomInGl.setDefaultKeybind({M_CTRL, "+"});
		iCamZoomOutGl.setDefaultKeybind({M_CTRL, "-"});
	}
};

// local ctor
ZoomTool::ZoomTool(std::tuple<ToolManager&, InputAdapter&> params)
: Tool(std::get<0>(params)),
  kb(std::make_unique<Keybinds>(std::get<1>(params).mkAdapter(getNameSt()))) {

	World& w = tm.getWorld();
	Camera& c = w.getCamera();
	SelfCursor& p = w.getCursor();

	kb->iSelectTool.setCb([this] (ImAction::Event& e, const InputInfo& ii) {
		tm.selectTool<ZoomTool>();
	});

	auto zoomInCb = [&] (auto&, const auto& ii) {
		float nz = c.getZoom() * 2.f;
		float wx = p.getFinalX();
		float wy = p.getFinalY();
		c.setZoom(nz >= 32.f ? 32.f : nz, wx, wy);
		w.recalculateCursorPosition(ii); // TODO: move this to camera
	};

	auto zoomOutCb = [&] (auto&, const auto& ii) {
		float nz = c.getZoom() / 2.f;
		float wx = p.getFinalX();
		float wy = p.getFinalY();
		c.setZoom(nz, wx, wy);
		w.recalculateCursorPosition(ii);
	};

	kb->iCamZoomIn.setCb(zoomInCb);
	kb->iCamZoomOut.setCb(zoomOutCb);
	kb->iCamZoomInGl.setCb(zoomInCb);
	kb->iCamZoomOutGl.setCb(zoomOutCb);

	kb->iCamZoomWhGl.setCb([&] (auto& ev, const auto& ii) {
		if (!(ii.getModifiers() & M_CTRL)) {
			ev.reject();
			return;
		}

		double d = ii.getWheelDy();
		if (d == 0.0) {
			return;
		}

		float nz = std::min(32.f, std::max(1.f, c.getZoom() - (d > 0.0 ? 1.f : -1.f)));
		float wx = p.getFinalX();
		float wy = p.getFinalY();
		c.setZoom(nz, wx, wy);
		w.recalculateCursorPosition(ii);
	});


	onSelectionChanged(false);
}

// remote ctor
ZoomTool::ZoomTool(ToolManager& tm)
: Tool(tm) { }

ZoomTool::~ZoomTool() { }

const char * ZoomTool::getNameSt() {
	return "Zoom";
}

std::string_view ZoomTool::getName() const {
	return getNameSt();
}

void ZoomTool::onSelectionChanged(bool selected) {
	if (!kb) {
		return;
	}

	kb->iCamZoomIn.setEnabled(selected);
	kb->iCamZoomOut.setEnabled(selected);
}

bool ZoomTool::isEnabled() {
	return true;
}

std::uint8_t ZoomTool::getNetId() const {
	return 0;
}

