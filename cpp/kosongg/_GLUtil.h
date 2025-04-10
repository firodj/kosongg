#pragma once

#include <glad/gl.h>

namespace kosongg {

void CheckGLError(const char *file, int line);

void GLAPIENTRY OnGLMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

bool LoadTextureFromFile(const char* filename, unsigned int* out_texture, int* out_width, int* out_height);

};
