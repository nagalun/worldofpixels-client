#pragma once

#include <cstdint>

#include <gl/program/ChunkProgram.hpp>

class LoadingChunkProgram : public ChunkProgram {
	std::int32_t uTime;

public:
	LoadingChunkProgram();

	void setUTime(float time);
};

