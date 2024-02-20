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

};
