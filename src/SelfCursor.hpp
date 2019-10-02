#pragma once

#include "Cursor.hpp"
#include "Bucket.hpp"

class SelfCursor : public Cursor {
	Bucket paintLimiter;
	Bucket chatLimiter;
	bool chatAllowed;
	bool modifyWorldAllowed;

public:
	SelfCursor(User&, Id, WorldPos, WorldPos, Step, Tid, Bucket paint, Bucket chat, bool canChat, bool canPaint);
	
	bool move(WorldPos, WorldPos, Step);
};
