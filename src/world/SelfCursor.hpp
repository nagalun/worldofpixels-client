#pragma once

#include <util/Bucket.hpp>
#include <world/Cursor.hpp>
#include <util/color.hpp>

class World;
class User;

class SelfCursor : public Cursor {
public:
	class Builder;

private:
	Bucket paintLimiter;
	Bucket chatLimiter;
	World& w;
	float preciseX;
	float preciseY;
	bool chatAllowed;
	bool modifyWorldAllowed;

public:
	SelfCursor(User&, Id, WorldPos, WorldPos, Step, Tid, World&, Bucket paint, Bucket chat, bool canChat, bool canPaint);

	float getFinalX() const;
	float getFinalY() const;
	void setPos(float, float);
	bool move(WorldPos, WorldPos, Step);
	bool paint(WorldPos, WorldPos, RGB_u);
};

class SelfCursor::Builder {
	User * usr;
	Id id;
	WorldPos spawnX;
	WorldPos spawnY;
	Step st;
	Tid tid;
	Bucket paint;
	Bucket chat;
	bool canChat;
	bool canPaint;

public:
	Builder();

	Builder& setCanChat(bool canChat);
	Builder& setCanPaint(bool canPaint);
	Builder& setId(Id id);
	Builder& setChatBucket(Bucket chat);
	Builder& setPaintBucket(Bucket paint);
	Builder& setSpawnX(WorldPos spawnX);
	Builder& setSpawnY(WorldPos spawnY);
	Builder& setStep(Step st);
	Builder& setToolId(Tid tid);
	Builder& setUser(User& usr);

	SelfCursor build(World& w);
};

