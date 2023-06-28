#pragma once

#include <cstdint>

#include "util/gl/Program.hpp"

#include <glm/vec2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class CursorProgram : public gl::Program {
	std::int32_t uMat;
	std::int32_t uAtlasTex;
	std::int32_t uAtlasSizePx;
	std::int32_t uWorldZoom;
	std::int32_t uDpr;
	glm::mat4 lastMat;
	float lastWorldZoom;
	float lastDpr;

public:
	CursorProgram();

	void setUMats(const glm::mat4& uProj, const glm::mat4& uView);
	void setUAtlasTex(std::int32_t sampler2D);
	void setUAtlasSizePx(float width, float height);
	void setUWorldZoom(float zoom);
	void setUDpr(float dpr);
};
