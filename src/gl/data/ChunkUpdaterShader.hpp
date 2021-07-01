#pragma once

#include <string_view>
#include <initializer_list>

struct ChunkUpdaterShader {
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
attribute vec2 vPosA;
attribute vec2 vPixelOffsetA;
attribute vec4 vPixelColorA;

varying vec4 vPixelColorV;

void main() {
	vPixelColorV = vPixelColorA;

	gl_Position = vec4(vPosA + vPixelOffsetA, 0.5, 1.0);
})"};

	static constexpr std::string_view fragment{
			R"(#version 100
precision mediump float;

varying vec4 vPixelColorV;

void main() {
	gl_FragColor = vPixelColorV;
})"};

	static constexpr std::initializer_list<const char *> attribs{
		"vPosA", "vPixelOffsetA", "vPixelColorA"
	};
};
