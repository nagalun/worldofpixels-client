#pragma once

#include <cstdint>

#include <util/gl/Program.hpp>

#include <glm/vec2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class ChunkProgram : public gl::Program {
	std::int32_t uMat;
	std::int32_t uZoom;
	std::int32_t uShowGrid;
	std::int32_t uChunkSize;
	std::int32_t uOffset;
	glm::mat4 lastMat;
	float lastZoom;
	bool lastShowGrid;

public:
	ChunkProgram(std::string_view fshdr);

	void setUShowGrid(bool show);
	void setUChunkSize(float uChunkSize);
	void setUOffset(glm::vec2 chunkOffset);
	void setUMats(const glm::mat4& uProj, const glm::mat4& uView);
	void setUZoom(float uZoom);
};
