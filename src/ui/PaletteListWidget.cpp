#include "PaletteListWidget.hpp"

#include "util/color.hpp"
#include "util/misc.hpp"

// "-2147483647"
constexpr std::size_t bufSz = 11;

PaletteListWidget::PaletteListWidget(ColorProvider& clr)
: clr(clr),
  worldPaletteContainer("details"),
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

	worldPaletteContainer.setAttribute("open");

	worldPaletteContainer.appendTo(*this);
	userPaletteContainer.appendTo(*this);

	setPalettes({{
		RGB_u{{26, 28, 44, 255}},
		RGB_u{{93, 39, 93, 255}},
		RGB_u{{177, 62, 83, 255}},
		RGB_u{{239, 125, 87, 255}},
		RGB_u{{255, 205, 117, 255}},
		RGB_u{{167, 240, 112, 255}},
		RGB_u{{56, 183, 100, 255}},
		RGB_u{{37, 113, 121, 255}},
		RGB_u{{41, 54, 111, 255}},
		RGB_u{{59, 93, 201, 255}},
		RGB_u{{65, 166, 246, 255}},
		RGB_u{{115, 239, 247, 255}},
		RGB_u{{244, 244, 244, 255}},
		RGB_u{{148, 176, 194, 255}},
		RGB_u{{86, 108, 134, 255}},
		RGB_u{{51, 60, 87, 255}}
	}}, {{
		RGB_u{{255, 255, 255, 255}},
		RGB_u{{255, 0, 0, 255}},
		RGB_u{{0, 255, 0, 255}},
		RGB_u{{0, 0, 255, 255}},
		RGB_u{{0, 0, 0, 255}}
	},{
		RGB_u{{127, 255, 127, 255}},
		RGB_u{{255, 255, 0, 255}},
		RGB_u{{0, 255, 255, 255}},
		RGB_u{{127, 255, 255, 255}}
	}});
}

void PaletteListWidget::setPalettes(std::vector<std::vector<RGB_u>> world, std::vector<std::vector<RGB_u>> user) {
	worldPalettes.clear();
	userPalettes.clear();
	worldPalettes.reserve(world.size());
	userPalettes.reserve(user.size());

	for (auto& palette : world) {
		auto& item = worldPalettes.emplace_back(clr, std::move(palette));
		item.appendTo(worldPaletteContainer);
	}

	for (auto& palette : user) {
		auto& item = userPalettes.emplace_back(clr, std::move(palette));
		item.appendTo(userPaletteContainer);
	}

	worldPaletteSummary.setAttribute("data-count", svprintf<bufSz>("%d", worldPalettes.size()));
	userPaletteSummary.setAttribute("data-count", svprintf<bufSz>("%d", userPalettes.size()));
}
