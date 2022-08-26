#include "PlayerCountWidget.hpp"

#include <util/misc.hpp>
#include <Renderer.hpp>

// "-2147483647"
constexpr std::size_t bufSz = 11;

PlayerCountWidget::PlayerCountWidget()
: painted(0),
  shownWorldCursorCount(0),
  shownGlobalCursorCount(0) {
	addClass("eui-wg");
	addClass("owop-cursor-count");
	setCounts(1, 1);
	appendToMainContainer();
}

void PlayerCountWidget::setCounts(std::uint32_t worldCursorCount, std::uint32_t globalCursorCount) {
	std::uint8_t shouldPaint = 0;
	if (worldCursorCount != shownWorldCursorCount) {
		shownWorldCursorCount = worldCursorCount;
		shouldPaint |= P_WCNT;
	}

	if (globalCursorCount != shownGlobalCursorCount) {
		shownGlobalCursorCount = globalCursorCount;
		shouldPaint |= P_GCNT;
	}

	if (shouldPaint != 0) {
		painted &= ~shouldPaint;
		Renderer::queueUiUpdateSt();
	}
}

void PlayerCountWidget::paint() {
	if (!(painted & P_WCNT)) {
		setAttribute("data-num-cur-world", svprintf<bufSz>("%d", shownWorldCursorCount));
	}

	if (!(painted & P_GCNT)) {
		setAttribute("data-num-cur-global", svprintf<bufSz>("%d", shownGlobalCursorCount));
	}
}
