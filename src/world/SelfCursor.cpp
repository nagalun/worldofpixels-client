#include "SelfCursor.hpp"

#include <cstdio>
#include <memory>

#include "PacketDefinitions.hpp"
#include "world/Cursor.hpp"
#include "world/World.hpp"
#include "Client.hpp"

SelfCursor::SelfCursor(User& u, Id id, WorldPos x, WorldPos y, Step s, Tid t, Tstate tstate, World& w, Bucket paint, Bucket chat, bool canChat, bool canPaint, u8 sseq, u8 aseq)
: Cursor(u, id, x, y, s, t),
  actionLimiter(paint),
  chatLimiter(chat),
  w(w),
  preciseX(x),
  preciseY(y),
  chatAllowed(canChat),
  modifyWorldAllowed(canPaint),
  needsSend(false),
  sseq(sseq),
  aseq(aseq) {
	// we can't call updateState on the toolManager yet, because it's still not initialized in world.
}

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
	needsSend |= Cursor::setPos(nx, ny);
}

bool SelfCursor::paint(WorldPos x, WorldPos y, RGB_u clr) {
	w.setPixel(x, y, clr, false);
	return true;
}

bool SelfCursor::update(
	WorldPos x, WorldPos y, Step step, Tid tid, Tstate tstate, Bucket nActionBkt, Bucket nChatBkt, u8 nSseq, u8 nAseq
) {

	bool updated = false;
	// this may not be optimal since the timestamp is not synchronized
	actionLimiter.set(nActionBkt.getRate(), nActionBkt.getPer(), nActionBkt.getAllowance());
	chatLimiter.set(nChatBkt.getRate(), nChatBkt.getPer(), nChatBkt.getAllowance());

	if (sseq != nSseq) {
		// client was teleported or is out of sync
		updated |= Cursor::update(x, y, step, w.getToolManager(), tid, tstate);
		w.getCamera().setPos(getFinalX(), getFinalY());
	}

	if (sseq != nSseq || aseq != nAseq) {
		// rollback/retry the unconfirmed actions. careful with overflow
		// aseq = 35
		// nAseq = 34
		// ov = 255 - (36 - 35)
		int actionsToRollback = nAseq > aseq ? 255 - (nAseq - aseq) : aseq - nAseq;
		std::printf("Need to roll back %d actions.\n", actionsToRollback);
		// TODO
	}

	sseq = nSseq;
	aseq = nAseq;
	return updated;
}

void SelfCursor::tick() {
	if (needsSend) {
		needsSend = false;
		sendState();
	}
}

void SelfCursor::markNeedsSend() {
	needsSend = true;
}

void SelfCursor::sendState() {
	auto& tm = w.getToolManager();
	std::uint64_t tstate = tm.getState(getToolStates());

	w.getClient().send(SPlayerUpdate::toBuffer({getId(), getX(), getY(), getStep(), getToolNetId(), tstate}, sseq));
}


SelfCursor::Builder::Builder()
: usr(nullptr),
  id(0),
  spawnX(0),
  spawnY(0),
  st(0),
  tid(0),
  tstate(0),
  action(0, 0),
  chat(0, 0),
  canChat(false),
  canPaint(false),
  sseq(0),
  aseq(0) { }

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

SelfCursor::Builder& SelfCursor::Builder::setActionBucket(Bucket nAction) {
	action = std::move(nAction);
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

SelfCursor::Builder& SelfCursor::Builder::setToolState(Tstate nTstate) {
	tstate = nTstate;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setUser(User& nUsr) {
	usr = std::addressof(nUsr);
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setStateSeq(u8 nSseq) {
	sseq = nSseq;
	return *this;
}

SelfCursor::Builder& SelfCursor::Builder::setActionSeq(u8 nAseq) {
	aseq = nAseq;
	return *this;
}

Cursor::Tid SelfCursor::Builder::getTid() const {
	return tid;
}

Cursor::Tstate SelfCursor::Builder::getTstate() const {
	return tstate;
}

SelfCursor SelfCursor::Builder::build(World& tw) {
	return SelfCursor(*usr, id, spawnX, spawnY, st, tid, tstate, tw,
			std::move(action), std::move(chat), canChat, canPaint, sseq, aseq);
}
