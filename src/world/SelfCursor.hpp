#pragma once

#include "util/Bucket.hpp"
#include "world/Cursor.hpp"
#include "util/color.hpp"
#include <vector>

class World;
class User;

class SelfCursor : public Cursor {
public:
	class Builder;

private:
	Bucket actionLimiter;
	Bucket chatLimiter;
	World& w;
	float preciseX;
	float preciseY;
	bool chatAllowed;
	bool modifyWorldAllowed;
	bool needsSend;
	u8 sseq;
	u8 aseq;

public:
	SelfCursor(User&, Id, WorldPos, WorldPos, Step, Tid, Tstate, World&, Bucket action, Bucket chat, bool canChat, bool canPaint, u8 sseq, u8 aseq);

	float getFinalX() const;
	float getFinalY() const;
	void setPos(float, float);
	bool move(WorldPos, WorldPos, Step);
	bool paint(WorldPos, WorldPos, RGB_u);
	bool update(
		WorldPos x, WorldPos y, Step step, Tid tid, Tstate tstate, Bucket actionBkt, Bucket chatBkt, u8 sseq, u8 aseq
	);
	void tick();
	void markNeedsSend();
	void sendState();
};

class SelfCursor::Builder {
	User * usr;
	Id id;
	WorldPos spawnX;
	WorldPos spawnY;
	Step st;
	Tid tid;
	Tstate tstate;
	Bucket action;
	Bucket chat;
	bool canChat;
	bool canPaint;
	u8 sseq;
	u8 aseq;

public:
	Builder();

	Builder& setCanChat(bool canChat);
	Builder& setCanPaint(bool canPaint);
	Builder& setId(Id id);
	Builder& setChatBucket(Bucket chat);
	Builder& setActionBucket(Bucket action);
	Builder& setSpawnX(WorldPos spawnX);
	Builder& setSpawnY(WorldPos spawnY);
	Builder& setStep(Step st);
	Builder& setToolId(Tid tid);
	Builder& setToolState(Tstate tstate);
	Builder& setUser(User& usr);
	Builder& setStateSeq(u8 sseq);
	Builder& setActionSeq(u8 aseq);

	Tid getTid() const;
	Tstate getTstate() const;

	SelfCursor build(World& w);
};
