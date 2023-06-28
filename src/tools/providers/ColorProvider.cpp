#include "ColorProvider.hpp"

#include <utility>

#include "ui/ColorWidget.hpp"
#include "InputManager.hpp"
#include "world/World.hpp"
#include "tools/ToolManager.hpp"
#include "util/NonCopyable.hpp"
#include "ui/PaletteListWidget.hpp"

ColorProvider::State::State()
: primaryColor{{0, 0, 0, 255}},
  secondaryColor{{255, 255, 255, 255}} { }

RGB_u ColorProvider::State::getPrimaryColor() const {
	return primaryColor;
}

RGB_u ColorProvider::State::getSecondaryColor() const {
	return secondaryColor;
}

bool ColorProvider::State::swapColors() {
	std::swap(primaryColor, secondaryColor);
	return primaryColor.rgb != secondaryColor.rgb;
}

bool ColorProvider::State::setPrimaryColor(RGB_u clr) {
	bool changed = clr.rgb != primaryColor.rgb;
	primaryColor = clr;
	return changed;
}

bool ColorProvider::State::setSecondaryColor(RGB_u clr) {
	bool changed = clr.rgb != secondaryColor.rgb;
	secondaryColor = clr;
	return changed;
}

struct ColorProvider::LocalContext : NonCopyable {
	ToolManager& tm;
	ColorProvider::State& localState;
	ColorWidget cw;
	PaletteListWidget paletteWdg;
	ImAction iSwapColors;
	decltype(ToolManager::onLocalStateChanged)::SlotKey cwUpdSk;

	LocalContext(ColorProvider& clr, ToolManager& _tm, InputAdapter& ia)
	: tm(_tm),
	  localState(tm.getLocalState().get<ColorProvider>()),
	  cw(tm, localState),
	  paletteWdg(clr),
	  iSwapColors(ia, "Swap Colors", T_ONPRESS) {
		iSwapColors.setDefaultKeybind("X");

		cw.setPaletteTglFn([this] {
			paletteWdg.tglClass("hide");
		});

		auto& llui = tm.getWorld().getLlCornerUi();
		auto& fstCol = llui.template get<0>();
		auto& sndCol = llui.template get<1>();
		cw.appendTo(fstCol);
		paletteWdg.appendTo(sndCol);

		iSwapColors.setCb([this] (auto&, const auto&) {
			if (localState.swapColors()) {
				tm.emitLocalStateChanged<ColorProvider>();
			}
		});

		cwUpdSk = tm.onLocalStateChanged.connect([this] (ToolStates& ts, Tool* cur) {
			cw.update();
		});
	}
};

// local ctor
ColorProvider::ColorProvider(std::tuple<ToolManager&, InputAdapter&> params)
: lctx(std::make_unique<LocalContext>(*this, std::get<0>(params), std::get<1>(params))) { }

ColorProvider::~ColorProvider() { }
