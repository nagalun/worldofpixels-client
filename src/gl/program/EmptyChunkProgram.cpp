#include "EmptyChunkProgram.hpp"

#include <gl/data/ChunkShader.hpp>

EmptyChunkProgram::EmptyChunkProgram()
: ChunkProgram(ChunkShader::emptyFragment) { }
