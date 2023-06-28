#include "ChunkProgram.hpp"

#include "gl/data/ChunkShader.hpp"

#include "world/Chunk.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLES2/gl2.h>

ChunkProgram::ChunkProgram(std::string_view fshdr)
: gl::Program(ChunkShader::vertex, fshdr, ChunkShader::attribs),
  uMat(findUniform("mat")),
  uZoom(findUniform("zoom")),
  uShowGrid(findUniform("showGrid")),
  uInvertClrs(findUniform("invertClrs")),
  uChunkSize(findUniform("chunkSize")),
  uOffset(findUniform("chunkOffset")),
  uBgClr(findUniform("bgClr")),
  lastBgClr(0.f),
  lastZoom(0.f),
  lastShowGrid(false),
  lastInvertClrs(false) {
	use();
	setUShowGrid(true);
	setUChunkSize(Chunk::size);
}

void ChunkProgram::setUShowGrid(bool show) {
	if (lastShowGrid != show) {
		glUniform1i(uShowGrid, show);
		lastShowGrid = show;
	}
}

void ChunkProgram::setUInvertColors(bool invert) {
	if (lastInvertClrs != invert) {
		glUniform1i(uInvertClrs, invert);
		lastInvertClrs = invert;
	}
}

void ChunkProgram::setUChunkSize(float chunkSize) {
	glUniform1f(uChunkSize, chunkSize);
}

void ChunkProgram::setUOffset(glm::vec2 chunkOffset) {
	glUniform2f(uOffset, chunkOffset.x, chunkOffset.y);
}

void ChunkProgram::setUBgClr(glm::vec3 bgClr) {
	if (lastBgClr != bgClr) {
		glUniform3f(uBgClr, bgClr.r, bgClr.g, bgClr.b);
		lastBgClr = bgClr;
	}
}

void ChunkProgram::setUMats(const glm::mat4& proj, const glm::mat4& view) {
	glm::mat4 newMat = proj * view;
	if (lastMat != newMat) {
		const float * f = glm::value_ptr(newMat);
		glUniformMatrix4fv(uMat, 1, GL_FALSE, f);
		lastMat = newMat;
	}
}

void ChunkProgram::setUZoom(float zoom) {
	if (lastZoom != zoom) {
		glUniform1f(uZoom, zoom);
		lastZoom = zoom;
	}
}
