#include "LoadingChunkProgram.hpp"

#include <gl/data/ChunkShader.hpp>

#include <GLES2/gl2.h>

LoadingChunkProgram::LoadingChunkProgram()
: ChunkProgram(ChunkShader::loadingFragment),
  uTime(findUniform("time")) { }

void LoadingChunkProgram::setUTime(float time) {
	glUniform1f(uTime, time);
}
