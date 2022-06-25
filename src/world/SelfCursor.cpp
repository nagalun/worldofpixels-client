#include <world/SelfCursor.hpp>

SelfCursor::SelfCursor(User& u, Id id, WorldPos x, WorldPos y, Step s, Tid t, Bucket paint, Bucket chat, bool canChat, bool canPaint)
: Cursor(u, id, x, y, s, t),
  paintLimiter(paint),
  chatLimiter(chat),
  preciseX(getFinalX()),
  preciseY(getFinalY()),
  chatAllowed(canChat),
  modifyWorldAllowed(canPaint) { }

bool SelfCursor::move(WorldPos x, WorldPos y, Step s) {
	return false;
}

float SelfCursor::getFinalX() const {
	return preciseX;
}

float SelfCursor::getFinalY() const {
	return preciseY;
}

void SelfCursor::setPos(float nx, float ny) {
	preciseX = nx;
	preciseY = ny;
	Cursor::setPos(nx, ny);
}
