#include "ChunkUpdaterGlState.hpp"

#include "util/explints.hpp"
#include "gl/data/ChunkUpdaterShader.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

ChunkUpdaterGlState::ChunkUpdaterGlState()
: verts(ChunkUpdaterShader::buffer) {
	vao.use();
	verts.use();

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr); // vPosA

	updateBuf.use();

	constexpr sz_t stride = 2 * sizeof(u16) + 4 * sizeof(u8);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, stride, nullptr); // vPixelOffsetA
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, reinterpret_cast<void*>(2 * sizeof(u16))); // vPixelColorA
	glVertexAttribDivisorANGLE(1, 1);
	glVertexAttribDivisorANGLE(2, 1);

	vao.enableAttribs(ChunkUpdaterShader::attribs.size());
}

bool ChunkUpdaterGlState::ok() const {
	return fb.get() && verts.get() && updateBuf.get() && cup.get() && vao.get();
}

void ChunkUpdaterGlState::use() {
	vao.use();
	cup.use();
	fb.use(GL_FRAMEBUFFER);
}

void ChunkUpdaterGlState::uploadPxData(const std::vector<PxUpdate>& data) {
	updateBuf.data(data.size() * sizeof(PxUpdate), data.data(), GL_DYNAMIC_DRAW);
}

void ChunkUpdaterGlState::uploadProtData(const std::vector<ProtUpdate>& data) {
	updateBuf.data(data.size() * sizeof(ProtUpdate), data.data(), GL_DYNAMIC_DRAW);
}

void ChunkUpdaterGlState::nullDataBuffer(sz_t size) {
	updateBuf.data(size, nullptr, GL_DYNAMIC_DRAW);
}
