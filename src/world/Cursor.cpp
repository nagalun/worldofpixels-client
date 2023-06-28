#include "world/Cursor.hpp"

#include "world/World.hpp"
#include "util/emsc/time.hpp"

#include <cmath>

Cursor::Cursor(User& usr, Id pid, WorldPos x, WorldPos y, Step st, Tid toolId)
: lastUpdate(getStClock()),
  usr(&usr),
  playerId(pid),
  x(x),
  y(y),
  smoothX((st & 0xF) / 15.f + x),
  smoothY((st >> 4 & 0xF) / 15.f + y),
  toolStates(toolId),
  pixelStep(st) { }

Cursor::Cursor(User& usr, Id pid, WorldPos x, WorldPos y, Step st, ToolManager& tm, Tid toolId, Tstate tstate)
: Cursor(usr, pid, x, y, st, toolId) {
	tm.updateState(toolStates, toolId, tstate);
}

Cursor::Cursor(Cursor&& o) noexcept
: lastUpdate(o.lastUpdate),
  usr(o.usr),
  playerId(o.playerId),
  x(o.x),
  y(o.y),
  smoothX(o.smoothX),
  smoothY(o.smoothY),
  toolStates(std::move(o.toolStates)),
  pixelStep(o.pixelStep) { }

Cursor& Cursor::operator=(Cursor&& o) noexcept {
	lastUpdate = o.lastUpdate;
	usr = o.usr;
	playerId = o.playerId;
	x = o.x;
	y = o.y;
	smoothX = o.smoothX;
	smoothY = o.smoothY;
	pixelStep = o.pixelStep;
	toolStates = std::move(o.toolStates);
	return *this;
}

WorldPos Cursor::getX() const {
	return x;
}

WorldPos Cursor::getY() const {
	return y;
}

twoi32 Cursor::getUpdArea() const {
	return World::updAreaOf(x, y);
}

Cursor::Step Cursor::getStep() const {
	return pixelStep;
}

float Cursor::getPosLerpTime() const {
	std::chrono::duration<float, std::milli> elapsed = getStClock() - lastUpdate;
	return std::min(elapsed.count() / World::updateRateMs, 1.f);
}

float Cursor::getSmoothX() const {
	return std::lerp(smoothX, getFinalX(), getPosLerpTime());
}

float Cursor::getSmoothY() const {
	return std::lerp(smoothY, getFinalY(), getPosLerpTime());
}

float Cursor::getFinalX() const {
	return (pixelStep & 0xF) / 15.f + x;
}

float Cursor::getFinalY() const {
	return (pixelStep >> 4 & 0xF) / 15.f + y;
}

User& Cursor::getUser() const {
	return *usr;
}

Cursor::Id Cursor::getId() const {
	return playerId;
}

Cursor::Tid Cursor::getToolNetId() const {
	return toolStates.getSelectedToolNetId();
}

const ToolStates& Cursor::getToolStates() const {
	return toolStates;
}

ToolStates& Cursor::getToolStates() {
	return toolStates;
}

bool Cursor::updateRel(WorldPos relX, WorldPos relY, Step newStep, ToolManager& tm, Tid newTid, Tstate newTstate) {
	return update(relX + getX(), relY + getY(), newStep, tm, newTid, newTstate);
}

bool Cursor::update(WorldPos newX, WorldPos newY, Step newStep, ToolManager& tm, Tid newTid, Tstate newTstate) {
	bool updated = false;
	updated |= setPos(newX, newY, newStep);
	updated |= tm.updateState(toolStates, newTid, newTstate);
	return updated;
}

bool Cursor::setPos(WorldPos newX, WorldPos newY, Step newPxStep) {
	smoothX = getSmoothX();
	smoothY = getSmoothY();

	lastUpdate = getStClock();

	bool updated = x != newX || y != newY || pixelStep != newPxStep;
	x = newX;
	y = newY;
	pixelStep = newPxStep;

	return updated;
}

bool Cursor::setPos(float worldX, float worldY) {
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
	stepX = std::round(stepX * 15.f);
	stepY = std::round(stepY * 15.f);

	WorldPos nx = worldX;
	WorldPos ny = worldY;
	Cursor::Step step = ((u8) (stepX) & 0xF) | (((u8) (stepY) & 0xF) << 4);

	return setPos(nx, ny, step);
}
