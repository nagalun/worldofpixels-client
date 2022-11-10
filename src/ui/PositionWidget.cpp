#include "PositionWidget.hpp"

#include "util/misc.hpp"
#include "world/World.hpp"
#include "Renderer.hpp"

PositionWidget::PositionWidget(World::Pos x, World::Pos y, float zoom)
: painted(0),
  shownMouseX(x - 1),
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
	std::uint8_t shouldPaint = 0;
	if (mx != shownMouseX) {
		shownMouseX = mx;
		shouldPaint |= P_MX;
	}

	if (my != shownMouseY) {
		shownMouseY = my;
		shouldPaint |= P_MY;
	}

	if (camx != shownCameraX) {
		shownCameraX = camx;
		shouldPaint |= P_CX;
	}

	if (camy != shownCameraY) {
		shownCameraY = camy;
		shouldPaint |= P_CY;
	}

	if (camZoom != shownZoom) {
		shownZoom = camZoom;
		shouldPaint |= P_ZOOM;
	}

	if (shouldPaint != 0) {
		painted &= ~shouldPaint;
		Renderer::queueUiUpdateSt();
	}
}

void PositionWidget::paint() {
	if (!(painted & P_MX)) {
		setAttribute("data-cur-x", svprintf("%d", shownMouseX));
		painted |= P_MX;
	}

	if (!(painted & P_MY)) {
		setAttribute("data-cur-y", svprintf("%d", shownMouseY));
		painted |= P_MY;
	}

	if (!(painted & P_CX)) {
		setAttribute("data-cam-x", svprintf("%d", shownCameraX));
		painted |= P_CX;
	}

	if (!(painted & P_CY)) {
		setAttribute("data-cam-y", svprintf("%d", shownCameraY));
		painted |= P_CY;
	}

	if (!(painted & P_ZOOM)) {
		auto zstr = svprintf("%.2f", shownZoom);
		if (zstr.compare(zstr.size() - 3, std::string_view::npos, ".00") == 0) {
			zstr.remove_suffix(3);
		}

		setAttribute("data-cam-zoom", zstr);
		painted |= P_ZOOM;
	}
}
