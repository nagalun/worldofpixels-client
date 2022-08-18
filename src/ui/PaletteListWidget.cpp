#include <ui/PaletteListWidget.hpp>

PaletteListWidget::PaletteListWidget()
: worldPaletteContainer("details"),
  worldPaletteSummary("summary"),
  userPaletteContainer("details"),
  userPaletteSummary("summary"),
  limitedPalettes(false) {
	addClass("eui-wg");
	addClass("owop-palettes");

	worldPaletteSummary.appendTo(worldPaletteContainer);
	userPaletteSummary.appendTo(userPaletteContainer);

	worldPaletteContainer.addClass("world-palettes");
	userPaletteContainer.addClass("user-palettes");

	worldPaletteContainer.appendTo(*this);
	userPaletteContainer.appendTo(*this);
}

