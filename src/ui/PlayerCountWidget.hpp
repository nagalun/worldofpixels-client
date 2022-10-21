#pragma once

#include <cstdint>
#include "util/emsc/ui/Object.hpp"

class PlayerCountWidget : public eui::Object {
	enum : std::uint8_t {
		P_WCNT = 1 << 1,
		P_GCNT = 1 << 2
	};

	std::uint8_t painted;
	std::uint32_t shownWorldCursorCount;
	std::uint32_t shownGlobalCursorCount;

public:
	PlayerCountWidget();

	void setCounts(std::uint32_t worldCursorCount, std::uint32_t globalCursorCount);
	void paint();
};

