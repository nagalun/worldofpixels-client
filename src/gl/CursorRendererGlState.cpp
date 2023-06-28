#include "CursorRendererGlState.hpp"

#include "ThemeManager.hpp"
#include "gl/data/CursorShader.hpp"

#include <cstddef>
#include <memory>

#include "util/explints.hpp"
#include "util/gl/Texture.hpp"
#include "util/misc.hpp"
#include "world/Cursor.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

CursorRendererGlState::CursorRendererGlState()
: verts(CursorShader::buffer),
  fxToolAtlas(nullptr) {
	vao.use();
	verts.use();

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr); // vPosA

	cursorData.use();

	constexpr auto offset = [] (std::size_t s) {
		return reinterpret_cast<void*>(s);
	};

	constexpr sz_t stride = 8 * sizeof(float);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, offset(0)); // vCamOffsetA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, offset(2 * sizeof(float))); // vAtlasToolTexPosA
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, offset(4 * sizeof(float))); // vAtlasToolTexSizeA
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, offset(6 * sizeof(float))); // vAtlasToolTexHotspotA
	glVertexAttribDivisorANGLE(1, 1);
	glVertexAttribDivisorANGLE(2, 1);
	glVertexAttribDivisorANGLE(3, 1);
	glVertexAttribDivisorANGLE(4, 1);

	vao.enableAttribs(CursorShader::attribs.size());

	program.use();
	program.setUAtlasTex(0);
}

bool CursorRendererGlState::ok() const {
	return verts.get() && cursorData.get() && program.get() && vao.get();
}

void CursorRendererGlState::use() {
	program.use();
	vao.use();
	if (fxToolAtlas.get()) {
		glActiveTexture(GL_TEXTURE0);
		fxToolAtlas.use(GL_TEXTURE_2D);
	}
}

bool CursorRendererGlState::uploadCurData(ToolManager& tm, const std::vector<Cursor>& cursors) {
	auto [buf, sz] = get_char_buf(cursors.size() * sizeof(float) * 8);
	float* flbuf = reinterpret_cast<float*>(/*std::assume_aligned<alignof(float)>(*/buf/*)*/);
	sz_t offs = 0;
	bool needsRender = false;

	Theme* t = ThemeManager::get().getCurrentTheme();
	if (!t || t->tools.empty()) {
		// skip rendering cursors if theme is not ready
		// but keep trying
		return true;
	}

	auto& imgAtlas = t->fxToolAtlas;
	float aw = imgAtlas.getWidth();
	float ah = imgAtlas.getHeight();
	if (!fxToolAtlas.get()) {
		fxToolAtlas = gl::Texture{};
		glActiveTexture(GL_TEXTURE0);
		fxToolAtlas.use(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		assert((imgAtlas.getChannels() == 4 && imgAtlas.getData() != nullptr)); // must be RGBA

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgAtlas.getWidth(), imgAtlas.getHeight(),
				0, GL_RGBA, GL_UNSIGNED_BYTE, imgAtlas.getData());

		program.use();
		program.setUAtlasSizePx(aw, ah);
	}

	auto* defTool = &t->tools[0];

	for (const Cursor& c : cursors) {
		auto& ts = c.getToolStates();

		// all these possible nulls are really ugly
		auto* tool = tm.getSelectedTool(ts);
		auto vname = tool ? tool->getToolVisualName(ts) : "";
		auto vstate = tool ? tool->getToolVisualState(ts) : 0;
		auto* tinfo = tool ? t->getToolInfo(vname) : defTool;
		auto& sinfo = tinfo->getState(vstate);

		// vCamOffsetA
		needsRender |= c.getPosLerpTime() != 1.f;
		flbuf[offs + 0] = c.getSmoothX();
		flbuf[offs + 1] = c.getSmoothY();
		// vAtlasToolTexPosA
		flbuf[offs + 2] = sinfo.fxAtlasX / aw;
		flbuf[offs + 3] = sinfo.fxAtlasY / ah;
		// vAtlasToolTexSizeA
		flbuf[offs + 4] = sinfo.fxAtlasW;
		flbuf[offs + 5] = sinfo.fxAtlasH;
		// vAtlasToolTexHotspotA
		flbuf[offs + 6] = sinfo.fxHotspotX;
		flbuf[offs + 7] = sinfo.fxHotspotY;
		offs += 8;
	}

	cursorData.use();
	cursorData.data(cursors.size() * sizeof(float) * 8, nullptr, GL_STREAM_DRAW);
	cursorData.data(cursors.size() * sizeof(float) * 8, static_cast<void*>(flbuf), GL_STREAM_DRAW);

	return needsRender;
}

CursorProgram& CursorRendererGlState::getProgram() {
	return program;
}
