#include <iostream>
#include <cassert>

#include "kosongg/GLUtil.h"

namespace kosongg {

void CheckGLError(const char *file, int line)
{
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR)
  {
    std::string error;

    switch(err) {

      case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
      case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
      case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
      case GL_STACK_OVERFLOW:         error="STACK_OVERFLOW";         break;
      case GL_STACK_UNDERFLOW:        error="STACK_UNDERFLOW";        break;
      case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
      case GL_CONTEXT_LOST:           error="GL_CONTEXT_LOST";        break;
      //case GL_TABLE_TOO_LARGE:        error="GL_TABLE_TOO_LARGE";     break;
    }

    std::cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line << std::endl;
    assert(err == GL_NO_ERROR);
  }
}

void GLAPIENTRY OnGLMessageCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam)
{
  std::cout << "---------------------opengl-callback-start------------" << std::endl;
  std::cout << "message: " << message << std::endl;
  std::cout << "type: ";
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "ERROR";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "DEPRECATED_BEHAVIOR";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "UNDEFINED_BEHAVIOR";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "PORTABILITY";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "PERFORMANCE";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "OTHER";
    break;
  default:
    std::cout << "0x" << std::hex << type;
  }
  std::cout << std::endl;

  std::cout << "id: 0x" << std::hex << id << std::endl;
  std::cout << "severity: ";
  switch (severity) {
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "LOW";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "MEDIUM";
    break;
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "HIGH";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    std::cout << "NOTIFICATION";
    break;
  default:
    std::cout << "0x" << std::hex << severity;
  }
  std::cout << std::endl;
  std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}

}