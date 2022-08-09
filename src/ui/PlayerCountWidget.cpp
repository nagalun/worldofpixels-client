#include "PlayerCountWidget.hpp"

#include <util/misc.hpp>

// "-2147483647"
constexpr std::size_t bufSz = 11;

PlayerCountWidget::PlayerCountWidget()
: shownWorldCursorCount(0),
  shownGlobalCursorCount(0) {
	addClass("eui-wg");
	addClass("owop-cursor-count");
	setCounts(1, 1);
	appendToMainContainer();
}

void PlayerCountWidget::setCounts(std::uint32_t worldCursorCount, std::uint32_t globalCursorCount) {
	if (worldCursorCount != shownWorldCursorCount) {
		setAttribute("data-num-cur-world", svprintf<bufSz>("%d", worldCursorCount));
		shownWorldCursorCount = worldCursorCount;
	}

	if (globalCursorCount != shownGlobalCursorCount) {
		setAttribute("data-num-cur-global", svprintf<bufSz>("%d", globalCursorCount));
		shownGlobalCursorCount = globalCursorCount;
	}
}
