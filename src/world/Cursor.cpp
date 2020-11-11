#include <world/Cursor.hpp>

Cursor::Cursor(User& usr, Id pid, WorldPos x, WorldPos y, Step st, Tid toolId)
: lastUpdate(std::chrono::steady_clock::now()),
  usr(usr),
  playerId(pid),
  x(x),
  y(y),
  smoothX(x + (st & 0xF) / 16.0),
  smoothY(y + (st >> 4 & 0xF) / 16.0),
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

double Cursor::getSmoothX() const {
	// TODO: actually smooth it
	return smoothX;
}

double Cursor::getSmoothY() const {
	return smoothY;
}

User& Cursor::getUser() const {
	return usr;
}

Cursor::Tid Cursor::getToolId() const {
	return toolId;
}
