#pragma once

#include <cstdint>
#include "util/emsc/ui/Object.hpp"


class PositionWidget : public eui::Object {
	using WorldPos = std::int32_t;

	enum : std::uint8_t {
		P_MX = 1 << 1,
		P_MY = 1 << 2,
		P_CX = 1 << 3,
		P_CY = 1 << 4,
		P_ZOOM = 1 << 5
	};

	std::uint8_t painted;
	WorldPos shownMouseX;
	WorldPos shownMouseY;
	WorldPos shownCameraX;
	WorldPos shownCameraY;
	float shownZoom;

public:
	PositionWidget(WorldPos x, WorldPos y, float zoom);

	void setPos(WorldPos mx, WorldPos my, WorldPos camx, WorldPos camy, float camZoom);
	void paint();
};

