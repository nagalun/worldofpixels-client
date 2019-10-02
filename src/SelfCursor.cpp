#include "SelfCursor.hpp"

SelfCursor::SelfCursor(User& u, Id id, WorldPos x, WorldPos y, Step s, Tid t, Bucket paint, Bucket chat, bool canChat, bool canPaint)
: Cursor(u, id, x, y, s, t),
  paintLimiter(std::move(paint)),
  chatLimiter(std::move(chat)),
  chatAllowed(canChat),
  modifyWorldAllowed(canPaint) { }

bool SelfCursor::move(WorldPos x, WorldPos y, Step s) {
	return false;
}
