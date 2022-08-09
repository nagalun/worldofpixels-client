#pragma once

#include <cstdint>
#include <util/emsc/ui/Object.hpp>

class PlayerCountWidget : public eui::Object {
	std::uint32_t shownWorldCursorCount;
	std::uint32_t shownGlobalCursorCount;

public:
	PlayerCountWidget();

	void setCounts(std::uint32_t worldCursorCount, std::uint32_t globalCursorCount);
};

