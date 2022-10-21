#pragma once

#include "util/explints.hpp"
#include <chrono>


using WorldPos = i32;

class User;

class Cursor {
public:
	using Id = u32;
	using Tid = u8; // ToolId
	using Step = u8; // Extra precision for X and Y

private:
	std::chrono::steady_clock::time_point lastUpdate;
	User& usr;
	const Id playerId;
	WorldPos x;
	WorldPos y;
	float smoothX;
	float smoothY;
	Step pixelStep;
	Tid toolId;

public:
	Cursor(User&, Id, WorldPos, WorldPos, Step, Tid);

	WorldPos getX() const;
	WorldPos getY() const;
	Step getStep() const;

	float getSmoothX() const;
	float getSmoothY() const;
	float getFinalX() const;
	float getFinalY() const;

	User& getUser() const;
	Tid getToolId() const;

	void setPos(WorldPos, WorldPos, Step);
	void setPos(float, float);
};
