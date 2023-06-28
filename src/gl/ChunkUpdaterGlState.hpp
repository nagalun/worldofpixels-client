#pragma once

#include <vector>

#include "util/explints.hpp"
#include "util/color.hpp"
#include "util/gl/ABuffer.hpp"
#include "util/gl/VtxArray.hpp"
#include "util/gl/Framebuffer.hpp"

#include "gl/program/ChunkUpdaterProgram.hpp"
#include "world/ChunkConstants.hpp"

class ChunkUpdaterGlState {
public:
	struct PxUpdate {
		// chunk-local pos
		u16 x;
		u16 y;
		RGB_u rgba;
	};

	struct ProtUpdate {
		// chunk-local pos
		u16 x;
		u16 y;
		ChunkConstants::ProtGid gid;
	};

private:
	gl::Framebuffer fb;
	gl::ABuffer verts;
	gl::ABuffer updateBuf;
	ChunkUpdaterProgram cup;
	gl::VtxArray vao;

public:
	ChunkUpdaterGlState();

	bool ok() const;

	void use();
	void uploadPxData(const std::vector<PxUpdate>&);
	void uploadProtData(const std::vector<ProtUpdate>&);
	void nullDataBuffer(sz_t nullSize);
};
