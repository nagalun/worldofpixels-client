#pragma once

#include "tools/ToolStates.hpp"
#include "util/explints.hpp"
#include "util/misc.hpp"
#include <chrono>
#include <string_view>


using WorldPos = i32;

class User;

class Cursor {
public:
	using Id = u32;
	using Tid = u8; // ToolId
	using Tstate = u64; // ToolState
	using Step = u8; // Extra precision for X and Y

private:
	std::chrono::steady_clock::time_point lastUpdate;
	User* usr;
	Id playerId;
	WorldPos x;
	WorldPos y;
	float smoothX;
	float smoothY;
	ToolStates toolStates;
	Step pixelStep;

public:
	Cursor(User&, Id, WorldPos, WorldPos, Step, Tid);
	Cursor(User&, Id, WorldPos, WorldPos, Step, ToolManager&, Tid, Tstate);

	Cursor(const Cursor&) = delete;
	Cursor& operator=(const Cursor&) = delete;
	Cursor(Cursor&& o) noexcept;
	Cursor& operator=(Cursor&&) noexcept;

	WorldPos getX() const;
	WorldPos getY() const;
	Step getStep() const;
	twoi32 getUpdArea() const;

	float getPosLerpTime() const;
	float getSmoothX() const;
	float getSmoothY() const;
	float getFinalX() const;
	float getFinalY() const;

	User& getUser() const;
	Id getId() const;
	Tid getToolNetId() const;
	const ToolStates& getToolStates() const;
	ToolStates& getToolStates();

	bool updateRel(WorldPos relX, WorldPos relY, Step, ToolManager&, Tid tid, Tstate tstate);
	bool update(WorldPos absX, WorldPos absY, Step, ToolManager&, Tid tid, Tstate tstate);
	bool setPos(WorldPos, WorldPos, Step);
	bool setPos(float, float);
};
