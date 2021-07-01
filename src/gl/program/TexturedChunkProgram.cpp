#include "TexturedChunkProgram.hpp"

#include <gl/data/ChunkShader.hpp>

#include <GLES2/gl2.h>

TexturedChunkProgram::TexturedChunkProgram()
: ChunkProgram(ChunkShader::texturedFragment),
  uPxTex(findUniform("pxTex")),
  uProtTex(findUniform("protTex")) {
	use();
	setUPxTex(0);
	setUProtTex(1);
}

void TexturedChunkProgram::setUPxTex(std::int32_t sampler2D) {
	glUniform1i(uPxTex, sampler2D);
}

void TexturedChunkProgram::setUProtTex(std::int32_t sampler2D) {
	glUniform1i(uProtTex, sampler2D);
}
