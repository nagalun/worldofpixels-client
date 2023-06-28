#include "CursorProgram.hpp"

#include "gl/data/CursorShader.hpp"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLES2/gl2.h>

CursorProgram::CursorProgram()
: gl::Program(CursorShader::vertex, CursorShader::fragment, CursorShader::attribs),
  uMat(findUniform("mat")),
  uAtlasTex(findUniform("atlasTex")),
  uAtlasSizePx(findUniform("atlasSizePx")),
  uWorldZoom(findUniform("worldZoom")),
  uDpr(findUniform("dpr")),
  lastWorldZoom(0.f),
  lastDpr(0.f) { }

void CursorProgram::setUMats(const glm::mat4& proj, const glm::mat4& view) {
	glm::mat4 newMat = proj * view;
	if (lastMat != newMat) {
		const float * f = glm::value_ptr(newMat);
		glUniformMatrix4fv(uMat, 1, GL_FALSE, f);
		lastMat = newMat;
	}
}

void CursorProgram::setUAtlasTex(std::int32_t sampler2D) {
	glUniform1i(uAtlasTex, sampler2D);
}

void CursorProgram::setUAtlasSizePx(float width, float height) {
	glUniform2f(uAtlasSizePx, width, height);
}

void CursorProgram::setUWorldZoom(float zoom) {
	if (zoom != lastWorldZoom) {
		glUniform1f(uWorldZoom, zoom);
		lastWorldZoom = zoom;
	}
}

void CursorProgram::setUDpr(float dpr) {
	if (dpr != lastDpr) {
		glUniform1f(uDpr, dpr);
		lastDpr = dpr;
	}
}
