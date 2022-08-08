#include "PositionWidget.hpp"

#include <util/misc.hpp>
#include <world/World.hpp>

// "-2147483647" "32.000000"
constexpr std::size_t bufSz = 11;

PositionWidget::PositionWidget(World::Pos x, World::Pos y, float zoom)
: shownMouseX(x - 1),
  shownMouseY(y - 1),
  shownCameraX(x - 1),
  shownCameraY(y - 1),
  shownZoom(zoom - 1.f) {
	addClass("eui-wg");
	addClass("owop-pos");
	setPos(x, y, x, y, zoom);
	appendToMainContainer();
}

void PositionWidget::setPos(World::Pos mx, World::Pos my, World::Pos camx, World::Pos camy, float camZoom) {
	if (mx != shownMouseX) {
		setAttribute("data-cur-x", svprintf<bufSz>("%d", mx));
		shownMouseX = mx;
	}

	if (my != shownMouseY) {
		setAttribute("data-cur-y", svprintf<bufSz>("%d", my));
		shownMouseY = my;
	}

	if (camx != shownCameraX) {
		setAttribute("data-cam-x", svprintf<bufSz>("%d", camx));
		shownCameraX = camx;
	}

	if (camy != shownCameraY) {
		setAttribute("data-cam-y", svprintf<bufSz>("%d", camy));
		shownCameraY = camy;
	}

	if (camZoom != shownZoom) {
		auto zstr = svprintf<bufSz>("%.2f", camZoom);
		if (zstr.compare(zstr.size() - 3, std::string_view::npos, ".00") == 0) {
			zstr.remove_suffix(3);
		}

		setAttribute("data-cam-zoom", zstr);
		shownZoom = camZoom;
	}
}
