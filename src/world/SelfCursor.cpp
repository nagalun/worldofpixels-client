#include "SelfCursor.hpp"

#include <memory>

#include "world/World.hpp"

SelfCursor::SelfCursor(User& u, Id id, WorldPos x, WorldPos y, Step s, Tid t, World& w, Bucket paint, Bucket chat, bool canChat, bool canPaint)
: Cursor(u, id, x, y, s, t),
  paintLimiter(paint),
  chatLimiter(chat),
  w(w),
  preciseX(x),
  preciseY(y),
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

bool SelfCursor::paint(WorldPos x, WorldPos y, RGB_u clr) {
	w.setPixel(x, y, clr, true);
	return true;
}



SelfCursor::Builder::Builder()
: usr(nullptr),
  id(0),
  spawnX(0),
  spawnY(0),
  st(0),
  tid(0),
  paint(0, 0),
  chat(0, 0),
  canChat(false),
  canPaint(false) { }

SelfCursor::Builder& SelfCursor::Builder::setCanChat(bool nCanChat) {
	canChat = nCanChat;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setCanPaint(bool nCanPaint) {
	canPaint = nCanPaint;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setId(Id nId) {
	id = nId;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setChatBucket(Bucket nChat) {
	chat = std::move(nChat);
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setPaintBucket(Bucket nPaint) {
	paint = std::move(nPaint);
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setSpawnX(WorldPos nSpawnX) {
	spawnX = nSpawnX;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setSpawnY(WorldPos nSpawnY) {
	spawnY = nSpawnY;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setStep(Step nSt) {
	st = nSt;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setToolId(Tid nTid) {
	tid = nTid;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setUser(User& nUsr) {
	usr = std::addressof(nUsr);
	return *this;
}

SelfCursor SelfCursor::Builder::build(World& tw) {
	return SelfCursor(*usr, id, spawnX, spawnY, st, tid, tw,
			std::move(paint), std::move(chat), canChat, canPaint);
}
