#include "PaletteItem.hpp"

#include <string_view>

#include "tools/providers/ColorProvider.hpp"
#include "util/byteswap.hpp"
#include "util/misc.hpp"

template<typename Fn>
PaletteItem::Color::Color(RGB_u color, Fn onSelect)
: Button(std::move(onSelect)),
  color(color) {
	addClass("owop-pal-clr");

	u32 cssClr = bswap_32(color.rgb);
	std::string_view cssStr;
	if (color.c.a == 255) {
		cssStr = svprintf("#%06X", cssClr >> 8);
	} else {
		cssStr = svprintf("#%08X", cssClr);
	}

	setAttribute("title", cssStr);
	setProperty("style.backgroundColor", cssStr);
}

RGB_u PaletteItem::Color::getColor() {
	return color;
}

PaletteItem::PaletteItem(ColorProvider& clrp, std::vector<RGB_u> clrs)
: clrp(clrp) {
	addClass("owop-palette");

	colors.reserve(clrs.size());

	for (RGB_u clr : clrs) {
		auto& clrElem = colors.emplace_back(clr, [this, clr] {
			this->clrp.setPrimaryColor(clr);
		});

		clrElem.appendTo(*this);
	}

	int numCols = 1;
	float nclrs = colors.size();

	if (nclrs > 8) {
		numCols++;
	}

	numCols += colors.size() / 32;

	float wPerc = 100.f / nclrs * numCols;

	setAttribute("data-name", "Test palette");
	setAttribute("data-clr-count", svprintf("%d", colors.size()));
	setProperty("style.--w-perc", svprintf("%f%%", wPerc));
}

PaletteItem::PaletteItem(PaletteItem&& o) noexcept
: eui::Object(std::move(o)),
  clrp(o.clrp),
  colors(std::move(o.colors)) {
	for (auto& clr : colors) {
		clr.setCb([this, clr{clr.getColor()}] {
			this->clrp.setPrimaryColor(clr);
		});
	}
}
