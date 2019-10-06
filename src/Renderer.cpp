#include "Renderer.hpp"

#include <exception>
#include <cstdio>
#include <cmath>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
//#include <glm/ext/matrix_transform.hpp>

#include "World.hpp"
#include "explints.hpp"

// TODO: use resume/pauseRendering()

constexpr std::string_view chunkVertex = R"(#version 100
uniform mediump mat4 view;
uniform mediump mat4 proj;

uniform vec2 chunkOffset;

attribute vec2 vPosA;
attribute vec2 vTexCoordA;

varying vec2 vTexCoordV;

void main() {
	vTexCoordV = vTexCoordA;

	gl_Position = proj * view * vec4(chunkOffset + vPosA, 0.5, 1.0);
}
)";

constexpr std::string_view chunkFragment = R"(#version 100
precision mediump float;

uniform mediump mat4 view;
uniform mediump mat4 projInv;
uniform highp vec2 chunkOffset;
uniform sampler2D tex;

varying vec2 vTexCoordV;

#define fmod(x,y) (x)-floor((x)/(y))*(y)

void main() {
	gl_FragColor = texture2D(tex, vTexCoordV);
	highp vec2 pos = (projInv * gl_FragCoord).xy;

    highp float yLine = fmod(pos.y + 0.0, 2.0);
    highp float xLine = fmod(pos.x + 0.0, 2.0);

    if((yLine >= -0.1 && yLine <= 0.1) || (xLine >= -0.1 && xLine <= 0.1)) {
        gl_FragColor = gl_FragColor * vec4(0.5, 0.5, 0.5, 1.0);
    }
}
)";

static GLint maxTexUnits;

static const GLint attrVPos = 0;
static const GLint attrVTexCoord = 1;

static const float test[] = {
	0.f,   0.f,   0.f, 0.f,
	0.f,   512.f, 0.f, 1.f,
	512.f, 0.f,   1.f, 0.f,
	512.f, 512.f, 1.f, 1.f,
	0.f,   512.f, 0.f, 1.f,
	512.f, 0.f,   1.f, 0.f
};

Renderer::Renderer(World& w)
: w(w),
  ctxInfo(0),
  vpWidth(1),
  vpHeight(1),
  view(1.0f),
  projection(1.0f),
  chunkProgram(0),
  fxProgram(0),
  cursorsProgram(0) {
	if (!activateRenderingContext()) {
		std::abort();
	}

	startRenderLoop();
	std::printf("[Renderer] Initialized\n");
}

Renderer::~Renderer() {
	stopRenderLoop();
	destroyRenderingContext();
	std::printf("[Renderer] Destroyed\n");
}

void Renderer::loadMissingChunks() {
	float hVpWidth = vpWidth / 2.f;
	float hVpHeight = vpHeight / 2.f;
	float tlx = std::floor((getX() - hVpWidth) / Chunk::size);
	float tly = std::floor((getY() - hVpHeight) / Chunk::size);
	float brx = std::floor((getX() + hVpWidth) / Chunk::size);
	float bry = std::floor((getY() + hVpHeight) / Chunk::size);

	for (; tly <= bry; tly += 1.f) {
		for (float tlx2 = tlx; tlx2 <= brx; tlx2 += 1.f) {
			w.getOrLoadChunk(tlx2, tly);
		}
	}
}

bool Renderer::isChunkVisible(Chunk& c) {
	float x = static_cast<float>(c.getX()) * Chunk::size;
	float y = static_cast<float>(c.getY()) * Chunk::size;
	float tlcx = getX() - vpWidth / 2.f;
	float tlcy = getY() - vpHeight / 2.f;
	float czoom = getZoom();

	return x + Chunk::size > tlcx && y + Chunk::size > tlcy &&
	       x <= tlcx + vpWidth / czoom && y <= tlcy + vpHeight / czoom;
}


void Renderer::useChunk(Chunk& c) {
	//std::printf("[Renderer] got chunk\n");
	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, c.getGlTexture());*/
	resumeRendering();
}

void Renderer::unuseChunk(Chunk& c) {

}

void Renderer::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float czoom = getZoom();
		//translate(0.f, 0.1f);

	/*if (getY() < 1200.f && getX() == 0.f) {
		translate(0.f, 1.0f);
		setZoom(czoom <= 1.f ? 1.f : czoom - 0.05f);
	} else if (getX() < 1200.f) {
		translate(1.0f, 0.f);
		setZoom(czoom <= 0.5f ? 0.5f : czoom - 0.025f);
	} else if (getY() > -1200.f) {
		translate(0.f, -1.0f);
		setZoom(czoom >= 2.0f ? 2.0f : czoom + 0.025f);
	} else {
		setX(0.0f);
		setZoom(16.f);
	}*/

	float hVpWidth = vpWidth / 2.f / czoom;
	float hVpHeight = vpHeight / 2.f / czoom;
	float tlx = std::floor((getX() - hVpWidth) / Chunk::size);
	float tly = std::floor((getY() - hVpHeight) / Chunk::size);
	float brx = std::floor((getX() + hVpWidth) / Chunk::size);
	float bry = std::floor((getY() + hVpHeight) / Chunk::size);

	view = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-getX(), -getY(), 0.f)), glm::vec3(1.f, 1.f, 1.f));
	setupProjection();

	glUseProgram(chunkProgram);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf);
	glEnableVertexAttribArray(attrVPos);
	glEnableVertexAttribArray(attrVTexCoord);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(chunkUnifTex, 0);
	glUniformMatrix4fv(chunkUnifView, 1, GL_FALSE, glm::value_ptr(view));
	updateUnifProjMatrix();
	for (; tly <= bry; tly += 1.f) {
		for (float tlx2 = tlx; tlx2 <= brx; tlx2 += 1.f) {
			Chunk& c = w.getOrLoadChunk(tlx2, tly);
			if (u32 tex = c.getGlTexture()) {
				glUniform2f(chunkUnifOffset, tlx2 * Chunk::size, tly * Chunk::size);
				glBindTexture(GL_TEXTURE_2D, tex);

				glDrawArrays(GL_TRIANGLES, 0, sizeof(test) / sizeof(float) / 4);
			}
		}
	}



	pauseRendering();
}

void Renderer::updateUnifViewMatrix() {
	glUseProgram(chunkProgram);
	glUniformMatrix4fv(chunkUnifView, 1, GL_FALSE, glm::value_ptr(view));
}

void Renderer::updateUnifProjMatrix() {
	glUseProgram(chunkProgram);
	glUniformMatrix4fv(chunkUnifProj, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(chunkUnifProjInv, 1, GL_FALSE, glm::value_ptr(projectionInv));
}

GLuint Renderer::buildProgram(std::string_view vertexShader, std::string_view fragmentShader) {
	GLuint vtx = glCreateShader(GL_VERTEX_SHADER);
	GLuint fmt = glCreateShader(GL_FRAGMENT_SHADER);

	if (!vtx || !fmt) {
		std::fprintf(stderr, "[Renderer] Failed to create shader(s)\n");
		glDeleteShader(vtx);
		glDeleteShader(fmt);
		return 0;
	}

	{
		const GLchar * vtxSrc[1] = {vertexShader.data()};
		const GLchar * fmtSrc[1] = {fragmentShader.data()};
		const GLint vtxLen[1]    = {static_cast<GLint>(vertexShader.size())};
		const GLint fmtLen[1]    = {static_cast<GLint>(fragmentShader.size())};

		glShaderSource(vtx, 1, vtxSrc, vtxLen);
		glShaderSource(fmt, 1, fmtSrc, fmtLen);
	}

	glCompileShader(vtx);
	glCompileShader(fmt);

	GLint result = 0;
	glGetShaderiv(vtx, GL_INFO_LOG_LENGTH, &result);
	if (result > 0) {
		GLchar msg[result];
		glGetShaderInfoLog(vtx, result, nullptr, msg);
		std::fputs(msg, stderr);
	}

	glGetShaderiv(fmt, GL_INFO_LOG_LENGTH, &result);
	if (result > 0) {
		GLchar msg[result];
		glGetShaderInfoLog(fmt, result, nullptr, msg);
		std::fputs(msg, stderr);
	}

	glGetShaderiv(vtx, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		std::fprintf(stderr, "[Renderer] Failed to compile vertex shader\n");
		glDeleteShader(vtx);
		glDeleteShader(fmt);
		return 0;
	}

	glGetShaderiv(fmt, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		std::fprintf(stderr, "[Renderer] Failed to compile fragment shader\n");
		glDeleteShader(vtx);
		glDeleteShader(fmt);
		return 0;
	}

	GLuint prog = glCreateProgram();
	if (!prog) {
		std::fprintf(stderr, "[Renderer] Failed to create gl program\n");
		glDeleteShader(vtx);
		glDeleteShader(fmt);
		return 0;
	}

	glAttachShader(prog, vtx);
	glAttachShader(prog, fmt);

	glBindAttribLocation(prog, attrVPos, "vPosA");
	glBindAttribLocation(prog, attrVTexCoord, "vTexCoordA");

	glLinkProgram(prog);

	glDetachShader(prog, vtx);
	glDetachShader(prog, fmt);
	glDeleteShader(vtx);
	glDeleteShader(fmt);

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &result);
	if (result > 0) {
		GLchar msg[result];
		glGetProgramInfoLog(prog, result, nullptr, msg);
		std::fputs(msg, stderr);
	}

	glGetProgramiv(vtx, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		std::fprintf(stderr, "[Renderer] Failed to link gl program\n");
		glDeleteProgram(prog);
		return 0;
	}

	return prog;
}


bool Renderer::setupView() {
	setZoom(16.f);

	//view = glm::lookAt(eye, center, up);
	return true;
}

bool Renderer::setupProjection() {
	float vpHalfW = vpWidth / 2.0f / getZoom();
	float vpHalfH = vpHeight / 2.0f / getZoom();
	float aspect = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);

	// left, right, bottom, top, near, far
	projection = glm::ortho(-vpHalfW, vpHalfW, vpHalfH, -vpHalfH, 0.5f, 1.5f);
	//projection = glm::frustum(-vpHalfW, vpHalfW, vpHalfH, -vpHalfH, 0.5f, 1.5f);
	//projection = glm::perspective(glm::radians(45.f), aspect, 0.5f, 1.5f);
	projection = glm::scale(projection, glm::vec3(1.f, 1.f, -1.f));

	projectionInv = glm::inverse(projection);
	//std::puts(glm::to_string(projection).c_str());
	return true;
}

bool Renderer::setupShaders() {
	chunkProgram = buildProgram(chunkVertex, chunkFragment);
	chunkUnifView    = glGetUniformLocation(chunkProgram, "view");
	chunkUnifProj    = glGetUniformLocation(chunkProgram, "proj");
	chunkUnifProjInv = glGetUniformLocation(chunkProgram, "projInv");
	chunkUnifOffset   = glGetUniformLocation(chunkProgram, "chunkOffset");
	chunkUnifTex     = glGetUniformLocation(chunkProgram, "tex");

	updateUnifViewMatrix();
	updateUnifProjMatrix();

	return chunkProgram;
}

bool Renderer::setupRenderingContext() {
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	std::printf("[Renderer] Max texture units: %i\n", maxTexUnits);
	glViewport(0, 0, vpWidth, vpHeight);

	RGB_u bgClr = w.getBackgroundColor();
	glClearColor(bgClr.r / 255.f, bgClr.g / 255.f, bgClr.b / 255.f, 0.5f);

	setupView();
	setupProjection();
	setupShaders();

	glGenBuffers(1, &tbuf);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(test), test, GL_STATIC_DRAW);

	glUseProgram(chunkProgram);
	glVertexAttribPointer(attrVPos, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

	glVertexAttribPointer(attrVTexCoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));


	return true;
}

void Renderer::doRender(void * r) {
	static_cast<Renderer *>(r)->render();
}
