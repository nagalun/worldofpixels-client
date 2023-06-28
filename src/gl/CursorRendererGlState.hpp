#pragma once

#include <vector>

#include "util/gl/ABuffer.hpp"
#include "util/gl/Texture.hpp"
#include "util/gl/VtxArray.hpp"

#include "gl/program/cursor/CursorProgram.hpp"

class Cursor;
class ToolManager;

class CursorRendererGlState {
	gl::ABuffer verts;
	gl::ABuffer cursorData;
	CursorProgram program;
	gl::VtxArray vao;
	gl::Texture fxToolAtlas;

public:
	CursorRendererGlState();

	bool ok() const;

	CursorProgram& getProgram();
	void use();
	bool uploadCurData(ToolManager& tm, const std::vector<Cursor>& cursors);
	std::size_t vertexCount();

};
