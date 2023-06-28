#include "PencilTool.hpp"

#include <cstdio>

#include "util/color.hpp"
#include "util/misc.hpp"
#include "InputManager.hpp"

#include "tools/ToolManager.hpp"
#include "tools/providers/ColorProvider.hpp"

#include "world/World.hpp"
#include "world/SelfCursor.hpp"

PencilTool::State::State()
: clicking(false),
  brushSize(1),
  brushShape(0),
  ditherType(0) { }

bool PencilTool::State::isClicking() const {
	return clicking;
}

std::uint8_t PencilTool::State::getDitherType() const {
	return ditherType;
}

std::uint8_t PencilTool::State::getBrushShape() const {
	return brushShape;
}

std::uint8_t PencilTool::State::getBrushSize() const {
	return brushSize;
}

bool PencilTool::State::setClicking(bool s) {
	bool changed = clicking != s;
	clicking = s;
	return changed;
}

bool PencilTool::State::setDitherType(std::uint8_t s) {
	bool changed = s != ditherType;
	ditherType = s;
	return changed;
}

bool PencilTool::State::setBrushShape(std::uint8_t s) {
	bool changed = s != brushShape;
	brushShape = s;
	return changed;
}

bool PencilTool::State::setBrushSize(std::uint8_t s) {
	bool changed = s != brushSize;
	brushSize = s;
	return changed;
}

struct PencilTool::LocalContext {
	ImAction iSelectTool;
	ImAction iDrawPrimaryClr;
	ImAction iDrawSecondaryClr;

	World::Pos lastX;
	World::Pos lastY;

	LocalContext(InputAdapter& ia)
	: iSelectTool(ia, "Select", T_ONPRESS),
	  iDrawPrimaryClr(ia, "Draw Primary Color", T_ONPRESS | T_ONMOVE | T_ONHOLD | T_ONRELEASE),
	  iDrawSecondaryClr(ia, "Draw Secondary Color", T_ONPRESS | T_ONMOVE | T_ONHOLD | T_ONRELEASE),
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
	ColorProvider::State& clr = tm.getLocalState().get<ColorProvider>();
	PencilTool::State& st = tm.getLocalState().get<PencilTool>();

	lctx->iSelectTool.setCb([this] (ImAction::Event& e, const InputInfo& ii) {
		tm.selectTool<PencilTool>();
	});

	const auto drawHandler = [&] (auto plotter) {
		return [&, plotter{std::move(plotter)}] (ImAction::Event& e, const InputInfo& ii) {
			int numActivePtrs = ii.getNumActivePointers();
			if (st.setClicking(numActivePtrs > 0)) {
				tm.emitLocalStateChanged<PencilTool>();
			}

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

PencilTool::~PencilTool() { }

const char * PencilTool::getNameSt() {
	return "Pencil";
}

std::string_view PencilTool::getName() const {
	return getNameSt();
}

std::string_view PencilTool::getToolVisualName(const ToolStates&) const {
	return "pencil";
}

void PencilTool::onSelectionChanged(bool selected) {
	lctx->iDrawPrimaryClr.setEnabled(selected);
	lctx->iDrawSecondaryClr.setEnabled(selected);
}

bool PencilTool::isEnabled() {
	return true;
}

std::uint8_t PencilTool::getNetId() const {
	return net::ToolId::TID_PENCIL;
}

// careful with endianness...
union NetState {
	struct __attribute__((packed)) {
		std::uint16_t primaryClr : 16;
		std::uint16_t primaryAlpha : 3;
		std::uint16_t clicking : 1;
		std::uint16_t brushSize : 4;
		std::uint16_t brushShape : 4;
		std::uint16_t ditherType : 4;
	} f;
	std::uint64_t data;
};

std::uint64_t PencilTool::getNetState(const ToolStates& ts) const {
	const auto& clrSt = ts.get<ColorProvider>();
	const auto& st = ts.get<PencilTool>();

	NetState netState{.data = 0};

	auto pClr = clrSt.getPrimaryColor();
	std::uint16_t pAlpha = pClr.c.a;

	netState.f.primaryClr = color_to_rgb565(pClr);
	netState.f.primaryAlpha = pAlpha * 7 / 255; // make it fit as 3 bits
	netState.f.clicking = st.isClicking();
	netState.f.brushSize = st.getBrushSize();
	netState.f.brushShape = st.getBrushShape();
	netState.f.ditherType = st.getDitherType();

	return netState.data;
}

bool PencilTool::setStateFromNet(ToolStates& ts, std::uint64_t netData) {
	auto& clrSt = ts.get<ColorProvider>();
	auto& st = ts.get<PencilTool>();

	NetState netState{.data = netData};
	bool updated = false;

	auto pClr = color_from_rgb565(netState.f.primaryClr);
	pClr.c.a = netState.f.primaryAlpha * 255 / 7;

	updated |= clrSt.setPrimaryColor(pClr);
	updated |= st.setClicking(netState.f.clicking);
	updated |= st.setBrushSize(netState.f.brushSize);
	updated |= st.setBrushShape(netState.f.brushShape);
	updated |= st.setDitherType(netState.f.ditherType);

	return updated;
}
