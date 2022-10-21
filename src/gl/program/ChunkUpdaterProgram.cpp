#include "ChunkUpdaterProgram.hpp"

#include "gl/data/ChunkUpdaterShader.hpp"

ChunkUpdaterProgram::ChunkUpdaterProgram()
: gl::Program(ChunkUpdaterShader::vertex, ChunkUpdaterShader::fragment, ChunkUpdaterShader::attribs) { }
