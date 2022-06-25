#include <world/Cursor.hpp>

#include <cmath>

Cursor::Cursor(User& usr, Id pid, WorldPos x, WorldPos y, Step st, Tid toolId)
: lastUpdate(std::chrono::steady_clock::now()),
  usr(usr),
  playerId(pid),
  x(x),
  y(y),
  smoothX((st & 0xF) / 16.0 + x),
  smoothY((st >> 4 & 0xF) / 16.0 + y),
  pixelStep(st),
  toolId(toolId) { }

WorldPos Cursor::getX() const {
	return x;
}

WorldPos Cursor::getY() const {
	return y;
}

Cursor::Step Cursor::getStep() const {
	return pixelStep;
}

float Cursor::getSmoothX() const {
	// TODO: actually smooth it
	return (pixelStep & 0xF) / 16.f + x;
}

float Cursor::getSmoothY() const {
	return (pixelStep >> 4 & 0xF) / 16.f + y;
}

float Cursor::getFinalX() const {
	// TODO: actually smooth it
	return (pixelStep & 0xF) / 16.f + x;
}

float Cursor::getFinalY() const {
	return (pixelStep >> 4 & 0xF) / 16.f + y;
}

User& Cursor::getUser() const {
	return usr;
}

Cursor::Tid Cursor::getToolId() const {
	return toolId;
}

void Cursor::setPos(WorldPos newX, WorldPos newY, Step newPxStep) {
	smoothX = getSmoothX();
	smoothY = getSmoothY();

	lastUpdate = std::chrono::steady_clock::now();
	x = newX;
	y = newY;
	pixelStep = newPxStep;
}

void Cursor::setPos(float worldX, float worldY) {
	float integ;
	float stepX = std::modf(worldX, &integ);
	float stepY = std::modf(worldY, &integ);
	worldX = std::floor(worldX);
	worldY = std::floor(worldY);

	if (stepX < 0.f) {
		stepX += 1.f;
	}

	if (stepY < 0.f) {
		stepY += 1.f;
	}

	// fixed precision
	stepX = std::round(stepX * 16.f);
	stepY = std::round(stepY * 16.f);

	WorldPos nx = worldX;
	WorldPos ny = worldY;
	Cursor::Step step = ((u8) (stepX) & 0xF) | (((u8) (stepY) & 0xF) << 4);

	setPos(nx, ny, step);
}
