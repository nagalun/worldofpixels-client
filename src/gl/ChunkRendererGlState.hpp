#pragma once

#include <cstddef>
#include <cstdint>

#include <util/gl/ABuffer.hpp>
#include <util/gl/Program.hpp>
#include <util/gl/VtxArray.hpp>

#include <gl/program/TexturedChunkProgram.hpp>
#include <gl/program/EmptyChunkProgram.hpp>
#include <gl/program/LoadingChunkProgram.hpp>

#include <glm/ext/matrix_float4x4.hpp>

class ChunkRendererGlState {
	gl::ABuffer verts;
	TexturedChunkProgram texturedProg;
	EmptyChunkProgram emptyProg;
	LoadingChunkProgram loadingProg;
	gl::VtxArray vao;

public:
	ChunkRendererGlState();

	void use();
	TexturedChunkProgram& getTexChunkProg();
	EmptyChunkProgram& getEmptyChunkProg();
	LoadingChunkProgram& getLoadChunkProg();
	std::size_t vertexCount();
};
