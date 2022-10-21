#include "gl/ChunkRendererGlState.hpp"
#include "gl/data/ChunkShader.hpp"
#include <GLES2/gl2.h>

ChunkRendererGlState::ChunkRendererGlState()
: verts(ChunkShader::buffer) {
	vao.use();
	verts.use();

	// vPosA
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	// vTexCoordA
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	vao.enableAttribs(ChunkShader::attribs.size());
}

void ChunkRendererGlState::use() {
	vao.use();
}

std::size_t ChunkRendererGlState::vertexCount() {
	return ChunkShader::buffer.size() / 4;
}

TexturedChunkProgram& ChunkRendererGlState::getTexChunkProg() {
	return texturedProg;
}

EmptyChunkProgram& ChunkRendererGlState::getEmptyChunkProg() {
	return emptyProg;
}

LoadingChunkProgram& ChunkRendererGlState::getLoadChunkProg() {
	return loadingProg;
}
