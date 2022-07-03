#pragma once

#include <string_view>
#include <initializer_list>

#include <world/ChunkConstants.hpp>

#define GLSL_GRID_FUNC " \
float gridMult() { \
	vec2 pixelPos = vTexCoordV * chunkSize * zoom; \
\
	float mult = 1.0; \
	float checker = float(mod(pixelPos.x, 2.0) < 1.0 ^^ mod(pixelPos.y, 2.0) < 1.0); \
\
	vec2 line1 = mod(pixelPos, zoom); \
	vec2 line16 = mod(pixelPos, 16.0 * zoom); \
\
	mult -= 0.2 * float(zoom > 2.0 && (line1.x < 1.0 || line1.y < 1.0)) * min(1.0, zoom / 2.0 - 1.0); \
	mult -= 0.2 * float(line16.x < 1.0 || line16.y < 1.0); \
	mult -= 0.2 * float(pixelPos.x < 1.0 || pixelPos.y < 1.0); \
\
	return mult + (checker * (1.0 - mult)); \
}"

struct ChunkShader {
	// vertex + texcoords
	static constexpr float chksz = ChunkConstants::size;
	static constexpr std::initializer_list<float> buffer{
		0.f,   0.f,   0.f, 0.f,
		0.f,   chksz, 0.f, 1.f,
		chksz, 0.f,   1.f, 0.f,
		chksz, chksz, 1.f, 1.f,
		0.f,   chksz, 0.f, 1.f,
		chksz, 0.f,   1.f, 0.f
	};

	static constexpr std::string_view vertex{
			R"(#version 100
uniform mediump mat4 mat;
uniform vec2 chunkOffset;

attribute vec2 vPosA;
attribute vec2 vTexCoordA;

varying vec2 vTexCoordV;

void main() {
	vTexCoordV = vTexCoordA;

	gl_Position = mat * vec4(chunkOffset + vPosA, 0.5, 1.0);
})"};

	static constexpr std::string_view texturedFragment{
			R"(#version 100
precision mediump float;

uniform mediump float chunkSize;
uniform mediump float zoom;
uniform sampler2D pxTex;
uniform sampler2D protTex;

varying vec2 vTexCoordV;

)" GLSL_GRID_FUNC R"(

void main() {
	vec4 texClr = texture2D(pxTex, vTexCoordV);
	float grid = gridMult();
	float alpha = 1.0 - grid * (1.0 - texClr.a);
	texClr.rgb *= texClr.a * grid / alpha;
	texClr.a = alpha;
	gl_FragColor = texClr;
})"}; // skipping grid color: (0.0 * grid / alpha) +

	static constexpr std::string_view emptyFragment{
			R"(#version 100
precision mediump float;

uniform mediump float chunkSize;
uniform mediump float zoom;

varying vec2 vTexCoordV;

)" GLSL_GRID_FUNC R"(

void main() {
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0 - gridMult());
})"};

	static constexpr std::string_view loadingFragment{
			R"(#version 100
precision mediump float;

uniform mediump float time;

uniform mediump float chunkSize;
uniform mediump float zoom;

varying vec2 vTexCoordV;

float twave(float i) {
	return abs(mod(i, 4.0) - 2.0) - 1.0;
}

float clamp1(float i) {
	return clamp(i, -1.0, 1.0);
}

float animx() {
	return (clamp1(twave(time) * 3.0) / 3.0 + 0.5) * 8.0 + 4.0;
}

float animy() {
	return (clamp1(twave(time + 1.0) * 3.0) / 3.0 + 0.5) * 8.0 + 4.0;
}

void main() {
	vec2 pixelPos = floor(vTexCoordV * chunkSize / (4.0 / zoom));

	float mult = float(
		(mod(pixelPos.x - pixelPos.y, 16.0) < animx())
		^^ (mod(pixelPos.x + pixelPos.y, 16.0) < animy())
	);

	float mult2 = (sin(time * 2.0) / 4.0) + 1.0;
	mult *= mult2;
	
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.15 + 0.05 * mult);
})"};

	static constexpr std::initializer_list<const char *> attribs{
		"vPosA", "vTexCoordA"
	};
};

#undef GLSL_GRID_FUNC
