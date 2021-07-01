#pragma once

#include <cstdint>

#include <gl/program/ChunkProgram.hpp>

class TexturedChunkProgram : public ChunkProgram {
	std::int32_t uPxTex;
	std::int32_t uProtTex;

public:
	TexturedChunkProgram();

	void setUPxTex(std::int32_t sampler2D);
	void setUProtTex(std::int32_t sampler2D);
};

