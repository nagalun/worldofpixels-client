#pragma once

#include <util/Bucket.hpp>
#include <world/Cursor.hpp>

class alignas(32) SelfCursor : public Cursor {
	Bucket paintLimiter;
	Bucket chatLimiter;
	bool chatAllowed;
	bool modifyWorldAllowed;

public:
	SelfCursor(User&, Id, WorldPos, WorldPos, Step, Tid, Bucket paint, Bucket chat, bool canChat, bool canPaint);

	bool move(WorldPos, WorldPos, Step);
};
