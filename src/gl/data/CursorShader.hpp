#pragma once

#include <initializer_list>
#include <string_view>

struct CursorShader {
	// vertex
	static constexpr std::initializer_list<float> buffer{
		0.f, 0.f,
		0.f, 1.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,
		1.f, 0.f
	};

	static constexpr std::string_view vertex{
			R"(#version 100
#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

uniform mat4 mat;
uniform vec2 atlasSizePx;
uniform float worldZoom;
uniform float dpr;

attribute vec2 vPosA;
attribute vec2 vCamOffsetA;
attribute vec2 vAtlasToolTexPosA;
attribute vec2 vAtlasToolTexSizeA;
attribute vec2 vAtlasToolTexHotspotA;

varying vec2 vTexCoordV;

void main() {
	float toolZoom = min(worldZoom / dpr, 16.0 / dpr);
	vTexCoordV = vAtlasToolTexPosA + vAtlasToolTexSizeA / atlasSizePx * vPosA;
	vec2 curPos = vCamOffsetA - vAtlasToolTexHotspotA / toolZoom;
	gl_Position = mat * vec4(curPos + vPosA * (vAtlasToolTexSizeA / toolZoom), 1.0, 1.0);
})"};

static constexpr std::string_view fragment{
			R"(#version 100
#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

uniform sampler2D atlasTex;

varying vec2 vTexCoordV;

void main() {
	gl_FragColor = texture2D(atlasTex, vTexCoordV);
})"};

	static constexpr std::initializer_list<const char *> attribs{
		"vPosA", "vCamOffsetA", "vAtlasToolTexPosA", "vAtlasToolTexSizeA", "vAtlasToolTexHotspotA"
	};

};
