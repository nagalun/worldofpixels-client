#pragma once

#include <cstdint>

#include "util/gl/Program.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class ChunkProgram : public gl::Program {
	std::int32_t uMat;
	std::int32_t uZoom;
	std::int32_t uShowGrid;
	std::int32_t uInvertClrs;
	std::int32_t uChunkSize;
	std::int32_t uOffset;
	std::int32_t uBgClr;
	glm::mat4 lastMat;
	glm::vec3 lastBgClr;
	float lastZoom;
	bool lastShowGrid;
	bool lastInvertClrs;

public:
	ChunkProgram(std::string_view fshdr);

	void setUShowGrid(bool show);
	void setUInvertColors(bool invert);
	void setUChunkSize(float uChunkSize);
	void setUOffset(glm::vec2 chunkOffset);
	void setUBgClr(glm::vec3 bgClr);
	void setUMats(const glm::mat4& uProj, const glm::mat4& uView);
	void setUZoom(float uZoom);
};
