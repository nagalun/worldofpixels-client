#include "Program.hpp"

#include <GLES2/gl2.h>
#include <cstdio>
#include <vector>
#include <utility>

namespace gl {

Program::Program(std::string_view vs, std::string_view fs,
		std::initializer_list<const char*> attribOrder)
: id(build(vs, fs, attribOrder)) { }

Program::~Program() {
	del();
}

Program::Program(Program&& p)
: id(std::exchange(p.id, 0)) { }

Program& Program::operator =(Program&& p) {
	del();
	id = std::exchange(p.id, 0);
	return *this;
}

void Program::use() const {
	glUseProgram(id);
}

std::uint32_t Program::get() const {
	return id;
}

std::int32_t Program::findUniform(const char * name) const {
	return glGetUniformLocation(id, name);
}

Program::operator bool() const {
	return id != 0;
}

std::uint32_t Program::build(std::string_view vs, std::string_view fs,
		std::initializer_list<const char*> attribOrder) {
	GLuint vtx = glCreateShader(GL_VERTEX_SHADER);
	GLuint fmt = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint prog = glCreateProgram();

	if (!vtx || !fmt || !prog) {
		std::fprintf(stderr, "[gl::Program] Failed to create shader(s)/program\n");
		glDeleteShader(vtx);
		glDeleteShader(fmt);
		glDeleteProgram(prog);
		return 0;
	}

	{
		const GLchar * vtxSrc[1] = {vs.data()};
		const GLchar * fmtSrc[1] = {fs.data()};
		const GLint vtxLen[1]    = {static_cast<GLint>(vs.size())};
		const GLint fmtLen[1]    = {static_cast<GLint>(fs.size())};

		glShaderSource(vtx, 1, vtxSrc, vtxLen);
		glShaderSource(fmt, 1, fmtSrc, fmtLen);
	}

	glCompileShader(vtx);
	glCompileShader(fmt);

	GLint result = 0;
	bool ok = true;
	for (GLuint sh : {vtx, fmt}) {
		glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &result);
		if (result > 1) {
			std::vector<GLchar> msg(result);
			glGetShaderInfoLog(sh, result, nullptr, msg.data());
			std::fputs(msg.data(), stderr);
		}

		glGetShaderiv(sh, GL_COMPILE_STATUS, &result);
		if (result != GL_TRUE) {
			std::fprintf(stderr, "[gl::Program] Failed to compile shader\n");
			glDeleteShader(sh);
			ok = false;
		}
	}

	if (!ok) {
		return 0;
	}

	glAttachShader(prog, vtx);
	glAttachShader(prog, fmt);

	std::size_t i = 0;
	for (auto it = attribOrder.begin(); it != attribOrder.end(); ++i, ++it) {
		glBindAttribLocation(prog, i, *it);
	}

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &result);
	if (result > 1) { /* Sometimes the driver returns a char for no reason, so ignore that */
		std::vector<GLchar> msg(result);
		glGetProgramInfoLog(prog, result, nullptr, msg.data());
		std::printf("P %i\n", result);
		std::fputs(msg.data(), stderr);
	}

	glGetProgramiv(prog, GL_LINK_STATUS, &result);
	glDetachShader(prog, vtx);
	glDetachShader(prog, fmt);

	if (result != GL_TRUE) {
		std::fprintf(stderr, "[gl::Program] Failed to link GL program\n");
		glDeleteProgram(prog);
		prog = 0;
	}

	glDeleteShader(vtx);
	glDeleteShader(fmt);

	return prog;
}

void Program::del() {
	if (id != 0) {
		glDeleteProgram(id);
	}
}

} /* namespace gl */
