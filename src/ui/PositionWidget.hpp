#pragma once

#include <cstdint>
#include <util/emsc/ui/Object.hpp>


class PositionWidget : public eui::Object {
	using WorldPos = std::int32_t;

	WorldPos shownMouseX;
	WorldPos shownMouseY;
	WorldPos shownCameraX;
	WorldPos shownCameraY;
	float shownZoom;

public:
	PositionWidget(WorldPos x, WorldPos y, float zoom);

	void setPos(WorldPos mx, WorldPos my, WorldPos camx, WorldPos camy, float camZoom);
};

